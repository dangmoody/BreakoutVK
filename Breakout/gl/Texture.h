#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "gl_main.h"

class VulkanContext;

enum samplerFilterMode_t {
	YETI_SAMPLER_FILTER_MODE_NEAREST	= 0,
	YETI_SAMPLER_FILTER_MODE_LINEAR,
};

struct samplerDesc_t {
	VkSampleCountFlagBits				mSampleCount;
	bool32								mEnableAnisotropicFiltering;

	samplerFilterMode_t					mFilterMode;
	VkSamplerAddressMode				mAddressMode;

	s32									mMipLevels;
	s32									mMinLod, mMaxLod;
};

struct textureDesc_t {
	u32									mWidth, mHeight, mDepth;

	VkFormat							mFormat;
};

/*
================================================================================================

	Yeti Texture

	Handles only exist on device local memory. Texture uploads happen via staging buffers.

	Yeti textures do not know (or care) about the pixel memory they currently store. The user
	(you) must handle this (if you want/need to).

================================================================================================
*/

class Texture {
public:
										Texture( VulkanContext* context );
										~Texture();

	inline bool32						IsAlloced() const { return mAllocation != VK_NULL_HANDLE; }

	inline const VkDescriptorImageInfo&	GetDescriptorInfo() const { return mDescriptorInfo; }

	void								AllocTexture( const textureDesc_t& textureDesc, const samplerDesc_t& samplerDesc );
	void								UnallocTexture();

	void								SubImageUpload2D( const u8* data, const s32 mipLevel, const s32 x, const s32 y, const s32 width, const s32 height );

	void								SetImageLayout( const VkCommandBuffer& commandBuffer, const VkImageLayout newLayout, const VkImageSubresourceRange& subresourceRange,
											const VkPipelineStageFlags sourceWaitFlags, const VkPipelineStageFlags destWaitFlags );

private:
	VmaAllocationInfo					mAllocInfo;

	VkDescriptorImageInfo				mDescriptorInfo;

	VmaAllocation						mAllocation;

	VkImage								mImage;

	VulkanContext*						mContext;

	s32									mMipLevels;
	s32									mBitsPerPixel;
};

#endif // __TEXTURE_H__