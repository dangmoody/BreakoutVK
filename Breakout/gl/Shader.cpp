#include "Shader.h"

/*
========================
Shader::Shader
========================
*/
Shader::Shader( VulkanContext* context ) {
	mContext = context;

	mShaderInfo = {};
	mShader = VK_NULL_HANDLE;
}

/*
========================
Shader::~Shader
========================
*/
Shader::~Shader() {
	if ( IsAlloced() ) {
		error( "Shader was never un-allocated! Please call UnallocShader() somewhere to do this!\n" );
	}
}

/*
========================
Shader::AllocShader
========================
*/
void Shader::AllocShader( const shaderDesc_t& desc ) {
	if ( IsAlloced() ) {
		error( "Cannot allocate Shader because it has already been allocated!\n" );
		return;
	}

	assertf( !desc.mFilename.empty(), "Invalid filename specified!" );

	char* buffer = nullptr;
	size_t bytes = readEntireFile( desc.mFilename.c_str(), &buffer );
	if ( bytes == 0 ) {
		error( "Unable to read shader file: %s\n", desc.mFilename.c_str() );
	}

	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = bytes;
	shaderInfo.pCode = reinterpret_cast<u32*>( buffer );
	YETI_VK_CHECK( vkCreateShaderModule( mContext->GetLogicalDevice(), &shaderInfo, nullptr, &mShader ) );

	mShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	mShaderInfo.module = mShader;
	mShaderInfo.pName = "main";
	mShaderInfo.stage = desc.mShaderStage;

	YETI_FREE( buffer );
}

/*
========================
Shader::UnallocShader
========================
*/
void Shader::UnallocShader() {
	if ( !IsAlloced() ) {
		return;
	}

	vkDestroyShaderModule( mContext->GetLogicalDevice(), mShader, nullptr );
	mShader = VK_NULL_HANDLE;
}