#include <assert.h>

#include "StagingManager.h"
#include "VulkanContext.h"

/*
================================================================================================

	StagingManager

================================================================================================
*/

/*
========================
StagingManager::StagingManager
========================
*/
StagingManager::StagingManager() {
	mMapped = nullptr;

	mDeviceMemory = VK_NULL_HANDLE;
	mCommandPool = VK_NULL_HANDLE;

	mInitialised = false;
}

/*
========================
StagingManager::~StagingManager
========================
*/
StagingManager::~StagingManager() {
	Shutdown();
}

/*
========================
StagingManager::Init
========================
*/
void StagingManager::Init( VulkanContext* context, const u32 numBuffers, const u32 maxBufferSizeMB ) {
	if ( IsInitialised() ) {
		error( "Attempt to call StagingManager::Init() when already initialised! Nothing will happen this time!\n" );
		return;
	}

#if MSTD_DEBUG
	timestamp_t start, end;
	float64 delta;

	printf( "------- Initialising Staging Manager -------\n" );

	start = timeNow();
#endif

	mContext = context;
	VkDevice logicalDevice = mContext->GetLogicalDevice();

	mStagingBuffers.resize( numBuffers );

	mMaxBufferSizeBytes = maxBufferSizeMB * MB_TO_BYTES;

	printf( "Allocating %d staging buffers with size %d MB each (%zu bytes).\n", numBuffers, maxBufferSizeMB, mMaxBufferSizeBytes );

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = mMaxBufferSizeBytes;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	for ( u32 i = 0; i < numBuffers; i++ ) {
		mStagingBuffers[i].mOffset = 0;

		YETI_VK_CHECK( vkCreateBuffer( logicalDevice, &bufferInfo, nullptr, &mStagingBuffers[i].mBuffer ) );
	}

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements( logicalDevice, mStagingBuffers[0].mBuffer, &memoryRequirements );

	VkDeviceSize alignedSize = GetAlignedSize( memoryRequirements.size, memoryRequirements.alignment );

	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = alignedSize * numBuffers;
	memoryAllocInfo.memoryTypeIndex = mContext->GetMemoryTypeIndex( VMA_MEMORY_USAGE_CPU_ONLY, memoryRequirements );
	YETI_VK_CHECK( vkAllocateMemory( logicalDevice, &memoryAllocInfo, nullptr, &mDeviceMemory ) );

	// re-get buffer memory requirements for buffer 0 here to avoid VK error because getting
	// buffer memory requirements is required before binding buffer memory (according to spec)
	for ( u32 i = 0; i < numBuffers; i++ ) {
		stagingBuffer_t& stagingBuffer = mStagingBuffers[i];
		vkGetBufferMemoryRequirements( logicalDevice, stagingBuffer.mBuffer, &memoryRequirements );

		YETI_VK_CHECK( vkBindBufferMemory( logicalDevice, stagingBuffer.mBuffer, mDeviceMemory, i * alignedSize ) );
	}

	assert( alignedSize > 0 );

	YETI_VK_CHECK( vkMapMemory( logicalDevice, mDeviceMemory, NULL, alignedSize * numBuffers, NULL, reinterpret_cast<void**>( &mMapped ) ) );

	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = mContext->GetQueueIndex( YETI_QUEUE_TYPE_GRAPHICS );
	YETI_VK_CHECK( vkCreateCommandPool( logicalDevice, &commandPoolInfo, nullptr, &mCommandPool ) );

	VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
	commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocInfo.commandBufferCount = 1;
	commandBufferAllocInfo.commandPool = mCommandPool;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	
	for ( u32 i = 0; i < numBuffers; i++ ) {
		stagingBuffer_t& stagingBuffer = mStagingBuffers[i];

		YETI_VK_CHECK( vkAllocateCommandBuffers( logicalDevice, &commandBufferAllocInfo, &stagingBuffer.mCommandBuffer ) );
		YETI_VK_CHECK( vkCreateFence( logicalDevice, &fenceInfo, nullptr, &stagingBuffer.mFence ) );
		YETI_VK_CHECK( vkBeginCommandBuffer( stagingBuffer.mCommandBuffer, &beginInfo ) );

		stagingBuffer.mData = reinterpret_cast<u8*>( mMapped + ( i * alignedSize ) );
	}

	mInitialised = true;

#if MSTD_DEBUG
	end = timeNow();
	delta = deltaMilliseconds( start, end );

	printf( "------- Staging Manager Initialized. Time Taken: %f ms -------\n", delta );
#endif
}

/*
========================
StagingManager::Shutdown
========================
*/
void StagingManager::Shutdown() {
	if ( !IsInitialised() ) {
		error( "Attempt to call StagingManager::Shutdown() was made when it's not initialized!\n" );
		return;
	}

#if MSTD_DEBUG
	printf( "------- Vulkan Staging Manager shutting down -------\n" );
#endif

	VkDevice logicalDevice = mContext->GetLogicalDevice();

	mContext->WaitDeviceIdle();

	vkUnmapMemory( logicalDevice, mDeviceMemory );
	mMapped = nullptr;
	
	for ( u32 i = 0; i < mStagingBuffers.length(); i++ ) {
		stagingBuffer_t& stagingBuffer = mStagingBuffers[i];

		vkDestroyFence( logicalDevice, stagingBuffer.mFence, nullptr );
		stagingBuffer.mFence = VK_NULL_HANDLE;

		vkFreeCommandBuffers( logicalDevice, mCommandPool, 1, &stagingBuffer.mCommandBuffer );
		stagingBuffer.mCommandBuffer = VK_NULL_HANDLE;

		vkDestroyBuffer( logicalDevice, stagingBuffer.mBuffer, nullptr );
		stagingBuffer.mBuffer = VK_NULL_HANDLE;
	}
	mStagingBuffers.clear();

	vkDestroyCommandPool( logicalDevice, mCommandPool, nullptr );
	mCommandPool = VK_NULL_HANDLE;

	vkFreeMemory( logicalDevice, mDeviceMemory, nullptr );
	mDeviceMemory = VK_NULL_HANDLE;

	mInitialised = false;

#if MSTD_DEBUG
	printf( "------- Vulkan Staging Manager shutdown -------\n" );
#endif
}

/*
========================
StagingManager::Stage
========================
*/
u8* StagingManager::Stage( const size_t sizeBytes, const s32 alignment, VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkDeviceSize& bufferOffset ) {
	assertf( sizeBytes <= mMaxBufferSizeBytes, "Stage() failed because specified size of data to stage was larger than the maximum allowed buffer size!\n" );
	assertf( alignment > 0, "Stage() failed because specified alignment was < 0! Must be >= 0!\n" );

	stagingBuffer_t& stagingBuffer = mStagingBuffers[mCurrentBufferIndex];
	stagingBuffer.mOffset = GetAlignedSize( sizeBytes, alignment );

	if ( ( stagingBuffer.mOffset + sizeBytes >= mMaxBufferSizeBytes ) && !stagingBuffer.mSubmitted ) {
		Flush();
	}

	stagingBuffer = mStagingBuffers[mCurrentBufferIndex];
	if ( stagingBuffer.mSubmitted ) {
		WaitForBuffer( stagingBuffer );
	}

	commandBuffer = stagingBuffer.mCommandBuffer;
	buffer = stagingBuffer.mBuffer;
	bufferOffset = stagingBuffer.mOffset;

	u8* data = stagingBuffer.mData + stagingBuffer.mOffset;
	stagingBuffer.mOffset += sizeBytes;

	return data;
}

/*
========================
StagingManager::Flush
========================
*/
void StagingManager::Flush() {
	stagingBuffer_t& stagingBuffer = mStagingBuffers[mCurrentBufferIndex];
	if ( stagingBuffer.mSubmitted || stagingBuffer.mOffset == 0 ) {
		return;
	}

	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT;
	vkCmdPipelineBarrier( stagingBuffer.mCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr );

	YETI_VK_CHECK( vkEndCommandBuffer( stagingBuffer.mCommandBuffer ) );

	VkMappedMemoryRange memoryRange = {};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = mDeviceMemory;
	memoryRange.size = VK_WHOLE_SIZE;
	YETI_VK_CHECK( vkFlushMappedMemoryRanges( mContext->GetLogicalDevice(), 1, &memoryRange ) );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &stagingBuffer.mCommandBuffer;
	YETI_VK_CHECK( vkQueueSubmit( mContext->GetQueue( YETI_QUEUE_TYPE_GRAPHICS ), 1, &submitInfo, stagingBuffer.mFence ) );

	stagingBuffer.mSubmitted = true;

	mCurrentBufferIndex = ( mCurrentBufferIndex + 1 ) % mStagingBuffers.length();
}

/*
========================
StagingManager::WaitForBuffer
========================
*/
void StagingManager::WaitForBuffer( stagingBuffer_t& stagingBuffer ) {
	if ( !stagingBuffer.mSubmitted ) {
		return;
	}

	YETI_VK_CHECK( vkWaitForFences( mContext->GetLogicalDevice(), 1, &stagingBuffer.mFence, VK_TRUE, U64_MAX ) );
	YETI_VK_CHECK( vkResetFences( mContext->GetLogicalDevice(), 1, &stagingBuffer.mFence ) );

	// reset staging buffer
	stagingBuffer.mOffset = 0;
	stagingBuffer.mSubmitted = false;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;	
	YETI_VK_CHECK( vkBeginCommandBuffer( stagingBuffer.mCommandBuffer, &beginInfo ) );
}