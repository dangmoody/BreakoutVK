#include "UniformLayout.h"
#include "VulkanContext.h"

/*
========================
UniformLayout::UniformLayout
========================
*/
UniformLayout::UniformLayout( VulkanContext* context ) {
	mContext = context;

	mDescriptorSet = VK_NULL_HANDLE;
	mDescriptorSetLayout = VK_NULL_HANDLE;
	mPipelineLayout = VK_NULL_HANDLE;
}

/*
========================
UniformLayout::~UniformLayout
========================
*/
UniformLayout::~UniformLayout() {
	if ( IsAlloced() ) {
		error( "UniformLayout was never un-allocated! Please call UnallocUniformLayout() somewhere to do this!\n" );
	}
}

/*
========================
UniformLayout::AllocUniformLayout
========================
*/
void UniformLayout::AllocUniformLayout( const uniformLayoutDesc_t& desc ) {
	if ( IsAlloced() ) {
		return;
	}

	bool32 hasUniformData = desc.mUniformBuffers.length() > 0 || desc.mTextures.length() > 0;
	if ( hasUniformData ) {
		assertf( desc.mDescriptorPool, "Cannot create a UniformLayout that has uniform buffers or textures specified without also specifying a VkDescriptorPool! Please make some!" );
		assertf( desc.mBindings, "Cannot create a UniformLayout that has uniform buffers or textures specified without also specifying VkDescriptorSetLayoutBindings! Please make some!" );
	}

	VkDevice device = mContext->GetLogicalDevice();

	VkDescriptorSetLayoutCreateInfo descSetLayoutInfo = {};
	descSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetLayoutInfo.bindingCount = desc.mNumBindings;
	descSetLayoutInfo.pBindings = desc.mBindings;
	YETI_VK_CHECK( vkCreateDescriptorSetLayout( device, &descSetLayoutInfo, nullptr, &mDescriptorSetLayout ) );

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pushConstantRangeCount = desc.mNumPushConstants;
	pipelineLayoutInfo.pPushConstantRanges = desc.mPushConstants;
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout;
	pipelineLayoutInfo.setLayoutCount = 1;
	YETI_VK_CHECK( vkCreatePipelineLayout( device, &pipelineLayoutInfo, nullptr, &mPipelineLayout ) );

	if ( hasUniformData ) {
		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.descriptorPool = desc.mDescriptorPool;
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &mDescriptorSetLayout;
		YETI_VK_CHECK( vkAllocateDescriptorSets( device, &descSetAllocInfo, &mDescriptorSet ) );
	}

	if ( hasUniformData ) {
		u32 uniformIndex = 0;
		u32 textureIndex = 0;

		array<VkWriteDescriptorSet> writeDescs( desc.mNumBindings );

		for ( u32 i = 0; i < desc.mNumBindings; i++ ) {
			VkWriteDescriptorSet& writeDesc = writeDescs[i];
			const VkDescriptorSetLayoutBinding& binding = desc.mBindings[i];

			writeDesc = {};
			writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDesc.descriptorCount = binding.descriptorCount;
			writeDesc.descriptorType = binding.descriptorType;
			writeDesc.dstBinding = binding.binding;
			writeDesc.dstSet = mDescriptorSet;

			switch ( binding.descriptorType ) {
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				writeDesc.pBufferInfo = &desc.mUniformBuffers[uniformIndex]->GetDescriptorInfo();
				uniformIndex++;
				break;

			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				writeDesc.pImageInfo = &desc.mTextures[textureIndex]->GetDescriptorInfo();
				textureIndex++;
				break;
			}
		}

		u32 numDescriptors = static_cast<u32>( writeDescs.length() );
		vkUpdateDescriptorSets( device, numDescriptors, writeDescs.data(), 0, nullptr );
	}
}

void UniformLayout::UnallocUniformLayout() {
	if ( !IsAlloced() ) {
		return;
	}

	VkDevice device = mContext->GetLogicalDevice();

	vkDestroyDescriptorSetLayout( device, mDescriptorSetLayout, nullptr );
	mDescriptorSetLayout = VK_NULL_HANDLE;

	vkDestroyPipelineLayout( device, mPipelineLayout, nullptr );
	mPipelineLayout = VK_NULL_HANDLE;
}