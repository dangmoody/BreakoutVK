#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "gl_main.h"

class VulkanContext;

struct bufferDesc_t {
	const void*								mData;
	size_t									mDataSizeBytes;
	VkBufferUsageFlagBits					mBufferUsage;
	VmaMemoryUsage							mMemoryUsage;
};

/*
================================================================================================

	Yeti Buffer

	Used to hold information being sent to/from the GPU. Knows the pointer to the raw data it
	holds as well as the mapped data. Also knows its alignment as well as optional CPU memory
	properties.

	All Buffers that are host visible get persistently mapped/unmapped automatically so there
	is no need to call Map()/Unmap().

================================================================================================
*/

class Buffer {
public:
											Buffer( VulkanContext* context );
											~Buffer();

	void									AllocBuffer( const bufferDesc_t& bufferDesc );
	void									UnallocBuffer();

	void									UploadData( const void* data, const size_t dataSizeBytes );
	void									Flush();

	inline const VkDescriptorBufferInfo&	GetDescriptorInfo() const { return mBufferInfo; }
	inline VkBuffer							GetAPIHandle() const { return mBufferInfo.buffer; }

	inline void*							GetMappedData() const { return mAllocInfo.pMappedData; }

	inline bool32							IsAlloced() const { return mAllocation != VK_NULL_HANDLE; }
	inline bool32							IsMapped() const { return mAllocInfo.pMappedData != nullptr; }

private:
	VmaAllocationInfo						mAllocInfo;

	VkDescriptorBufferInfo					mBufferInfo;

	VulkanContext*							mContext;

	VmaAllocation							mAllocation;

	VmaMemoryUsage							mMemoryUsage;
};

#endif // __BUFFER_H__