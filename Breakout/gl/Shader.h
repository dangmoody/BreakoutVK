#ifndef __SHADER_H__
#define __SHADER_H__

#include "gl_main.h"

struct shaderDesc_t {
	string							mFilename;
	VkShaderStageFlagBits			mShaderStage;
};

/*
================================================================================================

	Yeti Shader

	Keeps a copy of it's source code in a string as well as it's filename. neon will only
	read shaders that have been compiled offline into the .spv compiled format.

	Yeti shaders must be allocated separately from Yeti render states because it is possible
	for multiple render states to use the same shader but have other properties that are
	different.

================================================================================================
*/

class Shader {
public:
													Shader( VulkanContext* context );
													~Shader();

	void											AllocShader( const shaderDesc_t& desc );
	void											UnallocShader();

	inline bool32									IsAlloced() const { return mShader != VK_NULL_HANDLE; }

	inline const VkPipelineShaderStageCreateInfo&	GetShaderCreateInfo() const { return mShaderInfo; }

private:
	VkPipelineShaderStageCreateInfo					mShaderInfo;
	VkShaderModule									mShader;

	VulkanContext*									mContext;
};

#endif // __SHADER_H__