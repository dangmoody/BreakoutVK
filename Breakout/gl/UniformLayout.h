#ifndef __UNIFORM_H__
#define __UNIFORM_H__

#include "gl_main.h"

class Buffer;
class Texture;

struct uniformLayoutDesc_t {
	VkDescriptorPool				mDescriptorPool;

	array<Buffer*>					mUniformBuffers;
	array<Texture*>					mTextures;
	VkDescriptorSetLayoutBinding*	mBindings;
	VkPushConstantRange*			mPushConstants;

	u32								mNumBindings;
	u32								mNumPushConstants;
};

/*
================================================================================================

	Yeti Uniform Layout

	Holds information about all Uniform data inside a Yeti RenderState.

	It is still your job to allocate a VkDescriptorPool and pass this through to the
	uniformLayoutDesc_t upon creating the uniform layout.

================================================================================================
*/

class UniformLayout {
public:
									UniformLayout( VulkanContext* context );
									~UniformLayout();

	inline bool32					IsAlloced() const { return mPipelineLayout != VK_NULL_HANDLE; }

	inline VkDescriptorSet			GetDescriptorSet() const { return mDescriptorSet; }
	inline VkPipelineLayout			GetPipelineLayout() const { return mPipelineLayout; }

	void							AllocUniformLayout( const uniformLayoutDesc_t& desc );
	void							UnallocUniformLayout();

private:
	VulkanContext*					mContext;

	VkDescriptorSet					mDescriptorSet;
	VkDescriptorSetLayout			mDescriptorSetLayout;
	VkPipelineLayout				mPipelineLayout;
};

#endif // __UNIFORM_H__