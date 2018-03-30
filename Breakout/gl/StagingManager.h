#ifndef __STAGING_MANAGER_H__
#define __STAGING_MANAGER_H__

#include <mstd/mstd.h>

#include <vulkan/vulkan.h>
#include "vma/vma.h"

class VulkanContext;

/*
================================================================================================

	Staging Manager

	This is a modified implementation Dustin H. Land's "StagingManager" in his Vulkan port of
	DOOM 3 BFG. Absolutely no credit is taken for the implementation of this system or its
	architecture.

	To see the source implementation please refer to:
	https://github.com/DustinHLand/vkDOOM3/tree/master/neo/renderer/Vulkan

	Keeps a number of staging buffers equal to the number of buffers in the Swap Chain. When a
	request to the Staging Manager is made the Staging Manager returns a block of data of
	specified size, which can be written to directly and will get updated on the GPU straight
	away.

	All staging buffers and their memory are allocated on the same block to reduce number of
	individual allocations.

================================================================================================
*/

class StagingManager {
public:
	static const u32		MAX_BUFFER_SIZE_MB	= 64;

public:
							StagingManager();
	virtual					~StagingManager();

	void					Init( VulkanContext* renderBackend, const u32 numBuffers, const u32 maxBufferSizeMB = MAX_BUFFER_SIZE_MB );
	void					Shutdown();
	inline bool32			IsInitialised() const { return mInitialised; }

	u8*						Stage( const size_t sizeBytes, const s32 alignment, VkCommandBuffer& commandBuffer, VkBuffer& buffer, VkDeviceSize& bufferOffset );

	void					Flush();

private:
	struct stagingBuffer_t {
		u8*					mData				= nullptr;

		VkCommandBuffer		mCommandBuffer		= VK_NULL_HANDLE;
		VkBuffer			mBuffer				= VK_NULL_HANDLE;
		VkFence				mFence				= VK_NULL_HANDLE;
		VkDeviceSize		mOffset				= 0;

		bool32				mSubmitted = false;
	};

	u8*						mMapped;

	array<stagingBuffer_t>	mStagingBuffers;

	VulkanContext*			mContext;

	VkDeviceMemory			mDeviceMemory;

	VkCommandPool			mCommandPool;

	size_t					mMaxBufferSizeBytes;

	u32						mCurrentBufferIndex;

	bool32					mInitialised;

private:
	void					WaitForBuffer( stagingBuffer_t& stagingBuffer );

	// TODO: might need to move this somewhere more generic
	inline VkDeviceSize		GetAlignedSize( const VkDeviceSize memorySize, const VkDeviceSize alignment ) const;
};

/*
========================
StagingManager::GetAlignedSize
========================
*/
VkDeviceSize StagingManager::GetAlignedSize( const VkDeviceSize memorySize, const VkDeviceSize alignment ) const {
	const VkDeviceSize alignedSize = memorySize % alignment;
	return ( alignedSize == 0 ) ? memorySize : ( memorySize + alignment - alignedSize );
}

#endif // __STAGING_MANAGER_H__