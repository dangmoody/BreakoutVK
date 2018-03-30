#include "Texture.h"
#include "VulkanContext.h"

/*
================================================================================================

	Texture

================================================================================================
*/

/*
========================
Texture::Texture
========================
*/
Texture::Texture( VulkanContext* context ) {
	mContext = context;

	mAllocInfo = {};

	mDescriptorInfo = {};

	mAllocation = VK_NULL_HANDLE;

	mImage = VK_NULL_HANDLE;

	mMipLevels = 0;
	mBitsPerPixel = 0;
}

/*
========================
Texture::~Texture
========================
*/
Texture::~Texture() {
	if ( IsAlloced() ) {
		error( "Texture was never un-allocated! Please call UnallocTexture() somewhere to do this!\n" );
	}
}

/*
========================
Texture::AllocTexture
========================
*/
void Texture::AllocTexture( const textureDesc_t& textureDesc, const samplerDesc_t& samplerDesc ) {
	if ( IsAlloced() ) {
		error( "Cannot allocate Texture because it has already been allocated!\n" );
		return;
	}

	assertf( textureDesc.mWidth > 0, "Cannot create a texture with a width of 0! Must be > 0!\n" );
	assertf( textureDesc.mHeight > 0, "Cannot create a texture with a height of 0! Must be > 0!\n" );
	assertf( textureDesc.mDepth > 0, "Cannot create a texture with a depth of 0! Must be > 0!\n" );

	assertf( samplerDesc.mMipLevels > 0, "Cannot create a texture that has <= 0 mip levels! Must be > 0!\n" );
	assertf( samplerDesc.mMinLod >= 0, "Cannot create a texture that has < 0 min LoD! Must be > 0!\n" );
	assertf( samplerDesc.mMaxLod >= 1, "Cannot create a texture that has < 1 max LoD! Must be >= 1!\n" );

	mMipLevels = samplerDesc.mMipLevels;
	mBitsPerPixel = mContext->GetBitsPerPixelFromFormat( textureDesc.mFormat );

	VkSampleCountFlagBits sampleCount = static_cast<VkSampleCountFlagBits>( samplerDesc.mSampleCount );

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.arrayLayers = 1;
	imageInfo.extent = { textureDesc.mWidth, textureDesc.mHeight, textureDesc.mDepth };
	imageInfo.format = textureDesc.mFormat;
	imageInfo.imageType = ( textureDesc.mDepth > 1 ) ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.mipLevels = samplerDesc.mMipLevels;
	imageInfo.samples = sampleCount;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	YETI_VK_CHECK( vmaCreateImage( mContext->GetAllocator(), &imageInfo, &allocCreateInfo, &mImage, &mAllocation, &mAllocInfo ) );

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.components = {};
	imageViewInfo.format = imageInfo.format;
	imageViewInfo.image = mImage;
	imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageViewInfo.viewType = textureDesc.mDepth > 1 ? VK_IMAGE_VIEW_TYPE_3D : VK_IMAGE_VIEW_TYPE_2D;
	YETI_VK_CHECK( vkCreateImageView( mContext->GetLogicalDevice(), &imageViewInfo, nullptr, &mDescriptorInfo.imageView ) );

	VkSamplerMipmapMode mipmapMode;
	VkFilter filterMode;

	switch ( samplerDesc.mFilterMode ) {
	case YETI_SAMPLER_FILTER_MODE_NEAREST:
		mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		filterMode = VK_FILTER_NEAREST;
		break;

	case YETI_SAMPLER_FILTER_MODE_LINEAR:
		mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		filterMode = VK_FILTER_LINEAR;
		break;

	default:
		error( "Texture::AllocTexture() failed because invalid samplerFilterMode_t was passed!\n" );
		return;
	}

	// TODO: may need to make border color dependant on address mode
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.addressModeU = samplerDesc.mAddressMode;
	samplerInfo.addressModeV = samplerDesc.mAddressMode;
	samplerInfo.addressModeW = samplerDesc.mAddressMode;
	samplerInfo.anisotropyEnable = samplerDesc.mEnableAnisotropicFiltering;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.minFilter = filterMode;
	samplerInfo.magFilter = filterMode;
	samplerInfo.mipLodBias = 0;
	samplerInfo.mipmapMode = mipmapMode;
	samplerInfo.unnormalizedCoordinates = false;
	YETI_VK_CHECK( vkCreateSampler( mContext->GetLogicalDevice(), &samplerInfo, nullptr, &mDescriptorInfo.sampler ) );
}

/*
========================
Texture::UnallocTexture
========================
*/
void Texture::UnallocTexture() {
	if ( !IsAlloced() ) {
		return;
	}

	vkDestroySampler( mContext->GetLogicalDevice(), mDescriptorInfo.sampler, nullptr );
	mDescriptorInfo.sampler = VK_NULL_HANDLE;

	vkDestroyImageView( mContext->GetLogicalDevice(), mDescriptorInfo.imageView, nullptr );
	mDescriptorInfo.imageView = VK_NULL_HANDLE;

	vmaDestroyImage( mContext->GetAllocator(), mImage, mAllocation );
	mAllocation = VK_NULL_HANDLE;
	mImage = VK_NULL_HANDLE;
}

/*
========================
Texture::SubImageUpload2D
========================
*/
void Texture::SubImageUpload2D( const u8* data, const s32 mipLevel, const s32 x, const s32 y, const s32 width, const s32 height ) {
	assertf( data != nullptr, "Data was null! Cannot upload null texel data to a texture region!" );

	assertf( mipLevel >= 0, "MipLevel was < 0! Must be >= 0!" );
	assertf( mipLevel <= mMipLevels, "MipLevel was > texture's mip levels! Must be <= the texture's mip levels!" );

	assertf( x >= 0, "x of region was < 0! Must be >= 0!" );
	assertf( y >= 0, "y of region was < 0! Must be >= 0!" );

	assertf( width >= 0, "width of region was < 0! Must be >= 0!" );
	assertf( height >= 0, "height of region was < 0! Must be >= 0!" );

	u32 pitch = max( width, height );

	size_t pageSize = width * height * mBitsPerPixel / 8;
	VkDeviceSize bufferOffset = 0;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	u8* bufferMemory = mContext->GetStagingManager()->Stage( pageSize, YETI_DEFAULT_BYTE_ALIGNMENT, commandBuffer, stagingBuffer, bufferOffset );

	memcpy( bufferMemory, data, pageSize );

	VkBufferImageCopy imageCopy = {};
	imageCopy.imageOffset = { x, y, 0 };
	imageCopy.imageExtent = { static_cast<u32>( width ), static_cast<u32>( height ), 1U };
	imageCopy.bufferImageHeight = height;
	imageCopy.bufferOffset = bufferOffset;
	imageCopy.bufferRowLength = pitch;
	imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageCopy.imageSubresource.baseArrayLayer = 0; // z of 3D image region
	imageCopy.imageSubresource.layerCount = 1;
	imageCopy.imageSubresource.mipLevel = mipLevel;

	VkPipelineStageFlags sourceWaitFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destWaitFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	SetImageLayout( commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange, sourceWaitFlags, destWaitFlags );

	vkCmdCopyBufferToImage( commandBuffer, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy );

	sourceWaitFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	destWaitFlags = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	SetImageLayout( commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange, sourceWaitFlags, destWaitFlags );
}

/*
========================
Texture::SetImageLayout
========================
*/
void Texture::SetImageLayout( const VkCommandBuffer& commandBuffer, const VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange,
	const VkPipelineStageFlags sourceWaitFlags, const VkPipelineStageFlags destWaitFlags ) {

	if ( mDescriptorInfo.imageLayout == newLayout ) {
		error( "Texture::SetImageLayout() failed because current and target image layouts are the same!\n" );
		return;
	}

	VkImageMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memoryBarrier.image = mImage;
	memoryBarrier.oldLayout = mDescriptorInfo.imageLayout;
	memoryBarrier.newLayout = newLayout;
	memoryBarrier.subresourceRange = subresourceRange;
	memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	switch ( mDescriptorInfo.imageLayout ) {
	case VK_IMAGE_LAYOUT_UNDEFINED:
		memoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		memoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	default:
		error( "Texture::SetImageLayout() failed because the current image layout was invalid...\n" );
		return;
	}

	switch ( newLayout ) {
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if ( memoryBarrier.srcAccessMask == 0 ) {
			memoryBarrier.srcAccessMask |= VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;

	default:
		error( "Texture::SetImageLayout() failed because an unspported newLayout parm was given...\n" );
		return;
	}

	vkCmdPipelineBarrier( commandBuffer, sourceWaitFlags, destWaitFlags, VK_NULL_HANDLE, 0, nullptr, 0, nullptr, 1, &memoryBarrier );

	mDescriptorInfo.imageLayout = newLayout;
}