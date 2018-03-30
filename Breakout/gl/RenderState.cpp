#include "gl_main.h"

/*
========================
RenderState::RenderState
========================
*/
RenderState::RenderState( VulkanContext* context ) {
	mContext = context;

	mPipeline = VK_NULL_HANDLE;
}

/*
========================
RenderState::~RenderState
========================
*/
RenderState::~RenderState() {
	if ( IsAlloced() ) {
		error( "RenderSttae was never un-allocated! Please call UnallocRenderState() somewhere to do this!\n" );
	}
}

/*
========================
RenderState::AllocRenderState
========================
*/
void RenderState::AllocRenderState( const renderStateDesc_t& desc ) {
	if ( IsAlloced() ) {
		error( "Cannot allocate RenderState because it has already been allocated!\n" );
		return;
	}

	assertf( desc.mVertexShader, "Null vertex shader specified when trying to create RenderState! Please provide a valid Shader!\n" );
	assertf( desc.mFragmentShader, "Null fragment shader specified when trying to create RenderState! Please provide a valid Shader!\n" );

	assertf( desc.mVertexAttribs != nullptr, "Attempt to create a RenderState was made but specified VkVertexInputAttributeDescription was null!\n" );
	assertf( desc.mNumVertexAttribs > 0,
		"Attempt to create a RenderState was made but specified number of VkVertexInputAttributeDescriptions was 0! This must be the size of the vertex attributes array!\n" );

	assertf( desc.mVertexBindings != nullptr, "Attempt to create a RenderState was made but specified VkVertexInputBindingDescription was null!\n" );
	assertf( desc.mNumVertexBindings > 0,
		"Attempt to create a RenderState was made but specified number of VkVertexInputBindingDescriptions was 0! This must be the size of the vertex bindings array!\n" );

	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pVertexAttributeDescriptions = desc.mVertexAttribs;
	vertexInputState.vertexAttributeDescriptionCount = desc.mNumVertexAttribs;
	vertexInputState.pVertexBindingDescriptions = desc.mVertexBindings;
	vertexInputState.vertexBindingDescriptionCount = desc.mNumVertexBindings;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;
	inputAssemblyState.topology = desc.mTopology;

	VkPipelineRasterizationStateCreateInfo rasterState = {};
	rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterState.cullMode = desc.mCullMode;
	rasterState.depthClampEnable = VK_FALSE;
	rasterState.frontFace = desc.mFrontFace;
	rasterState.lineWidth = desc.mLineWidth;
	rasterState.polygonMode = desc.mPolygonMode;

	VkPipelineColorBlendAttachmentState blendAttachmentState = {};
	blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.blendEnable = desc.mEnableAlpha;
	if ( desc.mEnableAlpha ) {
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	}

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = desc.mEnableDepthTest;
	depthStencilState.depthCompareOp = desc.mDepthTestOp;
	depthStencilState.depthWriteEnable = desc.mEnableDepthWrite;
	depthStencilState.front = depthStencilState.back;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.scissorCount = 1;
	viewportState.viewportCount = 1;

	// TODO: implement SamplerState
	VkPipelineMultisampleStateCreateInfo multiSampleState = {};
	multiSampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multiSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	array<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = static_cast<u32>( dynamicStates.length() );
	dynamicStateInfo.pDynamicStates = dynamicStates.data();

	array<VkPipelineShaderStageCreateInfo> shaderStages = {
		desc.mVertexShader->GetShaderCreateInfo(),
		desc.mFragmentShader->GetShaderCreateInfo(),
	};

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.layout = desc.mUniformLayout->GetPipelineLayout();
	pipelineInfo.pColorBlendState = &colorBlendState;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineInfo.pMultisampleState = &multiSampleState;
	pipelineInfo.pRasterizationState = &rasterState;
	pipelineInfo.stageCount = static_cast<u32>( shaderStages.length() );
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.renderPass = mContext->GetRenderPass();
	YETI_VK_CHECK( vkCreateGraphicsPipelines( mContext->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mPipeline ) );
}

/*
========================
RenderState::UnallocRenderState
========================
*/
void RenderState::UnallocRenderState() {
	if ( !IsAlloced() ) {
		return;
	}

	vkDestroyPipeline( mContext->GetLogicalDevice(), mPipeline, nullptr );
	mPipeline = VK_NULL_HANDLE;
}