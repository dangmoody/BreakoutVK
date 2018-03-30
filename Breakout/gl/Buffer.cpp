#include "Buffer.h"

/*
================================================================================================

	Buffer

================================================================================================
*/

/*
========================
Buffer::Buffer
========================
*/
Buffer::Buffer( VulkanContext* context ) {
	mContext = context;

	mAllocInfo = {};

	mBufferInfo = {};

	mAllocation = VK_NULL_HANDLE;

	mMemoryUsage = VMA_MEMORY_USAGE_UNKNOWN;
}

/*
========================
Buffer::~Buffer
========================
*/
Buffer::~Buffer() {
	if ( IsAlloced() ) {
		error( "Buffer was never un-allocated! Please call UnallocBuffer() somewhere to do this!\n" );
	}
}

/*
========================
Buffer::AllocBuffer
========================
*/
void Buffer::AllocBuffer( const bufferDesc_t& desc ) {
	if ( IsAlloced() ) {
		error( "Cannot allocate buffer because it is already allocated!\n" );
		return;
	}

	mMemoryUsage = desc.mMemoryUsage;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = desc.mDataSizeBytes;
	bufferInfo.usage = desc.mBufferUsage;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = desc.mMemoryUsage;
	allocInfo.flags = VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT;

	YETI_VK_CHECK( vmaCreateBuffer( mContext->GetAllocator(), &bufferInfo, &allocInfo, &mBufferInfo.buffer, &mAllocation, &mAllocInfo ) );

	// used for uniform buffers when they are updated into the descriptor sets
	mBufferInfo.offset = 0;
	mBufferInfo.range = VK_WHOLE_SIZE;

	if ( desc.mMemoryUsage != VMA_MEMORY_USAGE_GPU_ONLY ) {
		if ( desc.mData ) {
			UploadData( desc.mData, desc.mDataSizeBytes );
		}
	}
}

/*
========================
Buffer::UnallocBuffer
========================
*/
void Buffer::UnallocBuffer() {
	if ( !IsAlloced() ) {
		return;
	}

	vmaDestroyBuffer( mContext->GetAllocator(), mBufferInfo.buffer, mAllocation );
	mAllocation = VK_NULL_HANDLE;
	mBufferInfo.buffer = VK_NULL_HANDLE;
}

/*
========================
Buffer::UploadData
========================
*/
void Buffer::UploadData( const void* data, const size_t dataSizeBytes ) {
	memcpy( mAllocInfo.pMappedData, data, dataSizeBytes );

	Flush();
}

/*
========================
Buffer::Flush
========================
*/
void Buffer::Flush() {
	// flush to make changes visible to host if memory is not host coherent
	if ( mMemoryUsage == VMA_MEMORY_USAGE_CPU_ONLY ) {
		VkMappedMemoryRange memoryRange = {};
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = mAllocInfo.deviceMemory;
		memoryRange.offset = mAllocInfo.offset;
		memoryRange.size = mAllocInfo.size;
		YETI_VK_CHECK( vkFlushMappedMemoryRanges( mContext->GetLogicalDevice(), 1, &memoryRange ) );
	}
}