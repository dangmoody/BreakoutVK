#include <SDL2/SDL.h>

#include <imgui/imgui.h>

#include "UI.h"
#include "Defines.h"
#include "Renderer.h"
#include "Window.h"
#include "Game.h"

#include "gl/gl_main.h"

/*
================================================================================================

	UI

================================================================================================
*/

UI* gUI = nullptr;

/*
========================
UI::UI
========================
*/
UI::UI() {
	mContext = nullptr;

	mBufferVertex = nullptr;
	mBufferIndex = nullptr;

	mShaderVertex = nullptr;
	mShaderFragment = nullptr;

	mRenderState = nullptr;
	mUniformLayout = nullptr;

	mDescriptorPool = VK_NULL_HANDLE;

	mBufferSizeVertex = 0;
	mBufferSizeIndex = 0;

	mNumVertices = mNumIndices = 0;

	mWidth = mHeight = 0;
	mWindowCounter = 0;

	mInitialised = false;
}

/*
========================
UI::~UI
========================
*/
UI::~UI() {
	Shutdown();
}

/*
========================
UI::Init
========================
*/
void UI::Init( const u32 screenWidth, const u32 screenHeight ) {
	if ( IsInitialised() ) {
		error( "Attempt to call UI::Init() when already initialised! Nothing will happen this time!\n" );
		return;
	}

	printf( "------- Initialising UI -------\n" );

	mWidth = screenWidth;
	mHeight = screenHeight;

	// imgui calls
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
	style.Colors[ImGuiCol_Header] = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
	style.WindowRounding = 0.0f;

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2( static_cast<float32>( screenWidth ), static_cast<float32>( screenHeight ) );
	io.FontGlobalScale = 1.0f;
	io.ImeWindowHandle = gWindow->GetHwnd();
	io.IniFilename = nullptr;	// disable the imgui.ini file

	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;

	u8* fontData;
	s32 textureWidth, textureHeight;
	io.Fonts->GetTexDataAsRGBA32( &fontData, &textureWidth, &textureHeight );

	mContext = gRenderer->GetContext();
	VkDevice device = mContext->GetLogicalDevice();

	// init shaders
	shaderDesc_t shaderDesc = {};
	shaderDesc.mFilename = BASE_PATH "shader_binaries/ui.vert.spv";
	shaderDesc.mShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
	mShaderVertex = new Shader( mContext );
	mShaderVertex->AllocShader( shaderDesc );

	shaderDesc.mFilename = BASE_PATH "shader_binaries/ui.frag.spv";
	shaderDesc.mShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	mShaderFragment = new Shader( mContext );
	mShaderFragment->AllocShader( shaderDesc );

	// init texture
	textureDesc_t fontTextureDesc = {};
	fontTextureDesc.mFormat = VK_FORMAT_R8G8B8A8_UNORM;
	fontTextureDesc.mWidth = textureWidth;
	fontTextureDesc.mHeight = textureHeight;
	fontTextureDesc.mDepth = 1;

	samplerDesc_t fontSamplerDesc = {};
	fontSamplerDesc.mAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	fontSamplerDesc.mEnableAnisotropicFiltering = false;
	fontSamplerDesc.mFilterMode = YETI_SAMPLER_FILTER_MODE_LINEAR;
	fontSamplerDesc.mMinLod = 0;
	fontSamplerDesc.mMaxLod = 1;
	fontSamplerDesc.mMipLevels = 1;
	fontSamplerDesc.mSampleCount = VK_SAMPLE_COUNT_1_BIT;

	mFontTexture = new Texture( mContext );
	mFontTexture->AllocTexture( fontTextureDesc, fontSamplerDesc );
	mFontTexture->SubImageUpload2D( fontData, 0, 0, 0, textureWidth, textureHeight );

	mBufferVertex = new Buffer( mContext );
	mBufferIndex = new Buffer( mContext );

	// init uniform resource data
	{
		array<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.maxSets = 1;
		descriptorPoolInfo.poolSizeCount = static_cast<u32>( poolSizes.length() );
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		YETI_VK_CHECK( vkCreateDescriptorPool( device, &descriptorPoolInfo, nullptr, &mDescriptorPool ) );

		array<VkDescriptorSetLayoutBinding> descSetBindings = {
			{ 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
		};

		array<Texture*> textures = {
			mFontTexture,
		};

		VkPushConstantRange pushConstantRange = {
			VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( pushConstantBlock_t )
		};

		uniformLayoutDesc_t uniformLayoutDesc = {};
		uniformLayoutDesc.mDescriptorPool = mDescriptorPool;
		uniformLayoutDesc.mBindings = descSetBindings.data();
		uniformLayoutDesc.mNumBindings = static_cast<u32>( descSetBindings.length() );
		uniformLayoutDesc.mTextures = textures;
		uniformLayoutDesc.mPushConstants = &pushConstantRange;
		uniformLayoutDesc.mNumPushConstants = 1;
		mUniformLayout = new UniformLayout( mContext );
		mUniformLayout->AllocUniformLayout( uniformLayoutDesc );

		array<VkVertexInputBindingDescription> vertexBindings = {
			{ 0, sizeof( ImDrawVert ), VK_VERTEX_INPUT_RATE_VERTEX },
		};

		array<VkVertexInputAttributeDescription> vertexAttribs = {
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( ImDrawVert, pos ) },
			{ 1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof( ImDrawVert, uv ) },
			{ 2, 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof( ImDrawVert, col ) },
		};

		renderStateDesc_t renderStateDesc = {};
		renderStateDesc.mCullMode = VK_CULL_MODE_NONE;
		renderStateDesc.mDepthTestOp = VK_COMPARE_OP_NEVER;
		renderStateDesc.mEnableAlpha = VK_TRUE;
		renderStateDesc.mEnableDepthTest = VK_FALSE;
		renderStateDesc.mEnableDepthWrite = VK_FALSE;
		renderStateDesc.mEnableStencil = VK_FALSE;
		renderStateDesc.mFrontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		renderStateDesc.mVertexShader = mShaderVertex;
		renderStateDesc.mFragmentShader = mShaderFragment;
		renderStateDesc.mLineWidth = 1.0f;
		renderStateDesc.mNumVertexBindings = static_cast<u32>( vertexBindings.length() );
		renderStateDesc.mVertexBindings = vertexBindings.data();
		renderStateDesc.mNumVertexAttribs = static_cast<u32>( vertexAttribs.length() );
		renderStateDesc.mVertexAttribs = vertexAttribs.data();
		renderStateDesc.mPolygonMode = VK_POLYGON_MODE_FILL;
		renderStateDesc.mTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		renderStateDesc.mUniformLayout = mUniformLayout;
		mRenderState = new RenderState( mContext );
		mRenderState->AllocRenderState( renderStateDesc );
	}

	mInitialised = true;

	printf( "------- UI initialised -------\n\n" );
}

/*
========================
UI::Shutdown
========================
*/
void UI::Shutdown() {
	if ( !IsInitialised() ) {
		error( "Attempt to call UI::Shutdown() was made when it's not initialized!\n" );
		return;
	}

	printf( "------- UI shutting down -------\n" );

	mContext->WaitDeviceIdle();

	VkDevice device = mContext->GetLogicalDevice();

	vkDestroyDescriptorPool( device, mDescriptorPool, nullptr );
	mDescriptorPool = VK_NULL_HANDLE;

	mUniformLayout->UnallocUniformLayout();
	YETI_FREE( mUniformLayout );

	mRenderState->UnallocRenderState();
	YETI_FREE( mRenderState );

	mFontTexture->UnallocTexture();
	YETI_FREE( mFontTexture );

	mShaderFragment->UnallocShader();
	YETI_FREE( mShaderFragment );

	mShaderVertex->UnallocShader();
	YETI_FREE( mShaderVertex );

	mBufferIndex->UnallocBuffer();
	YETI_FREE( mBufferIndex );

	mBufferVertex->UnallocBuffer();
	YETI_FREE( mBufferVertex );

	mInitialised = false;

	printf( "------- UI shutdown -------\n\n" );
}

/*
========================
UI::PushWindow
========================
*/
void UI::PushWindow( const ImVec2& pos, const ImVec4& backgroundColor ) {
	ImGui::PushStyleColor( ImGuiCol_WindowBg, backgroundColor );
	ImGui::SetNextWindowPos( pos );
	ImGui::SetNextWindowSize( ImVec2( 0, 0 ), ImGuiCond_FirstUseEver );

	string id = "Window" + toString( mWindowCounter++ );
	ImGui::Begin( id.c_str(), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize );
}

/*
========================
UI::PopWindow
========================
*/
void UI::PopWindow() {
	ImGui::End();
	ImGui::PopStyleColor();
}

/*
========================
UI::Begin
========================
*/
void UI::Begin() {
	mWindowCounter = 0;

	ImGui::NewFrame();
}

/*
========================
UI::End
========================
*/
void UI::End() {
	ImGui::Render();
}

/*
========================
UI::Render
========================
*/
void UI::Render() {
	ImDrawData* drawData = ImGui::GetDrawData();
	if ( !drawData ) {
		return;
	}

	bufferDesc_t bufferDesc = {};

	size_t bufferSizeVertex = drawData->TotalVtxCount * sizeof( ImDrawVert );
	if ( mBufferSizeVertex != bufferSizeVertex ) {
		mContext->WaitDeviceIdle();

		mBufferVertex->UnallocBuffer();

		bufferDesc.mBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferDesc.mData = nullptr;
		bufferDesc.mDataSizeBytes = bufferSizeVertex;
		bufferDesc.mMemoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		mBufferVertex->AllocBuffer( bufferDesc );

		mBufferSizeVertex = bufferSizeVertex;
		mNumVertices = drawData->TotalVtxCount;
	}

	size_t bufferSizeIndex = drawData->TotalIdxCount * sizeof( ImDrawIdx );
	if ( mBufferSizeIndex != bufferSizeIndex ) {
		mContext->WaitDeviceIdle();

		mBufferIndex->UnallocBuffer();

		bufferDesc.mBufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		bufferDesc.mData = nullptr;
		bufferDesc.mDataSizeBytes = bufferSizeIndex;
		bufferDesc.mMemoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		mBufferIndex->AllocBuffer( bufferDesc );

		mBufferSizeIndex = bufferSizeIndex;
		mNumIndices = drawData->TotalIdxCount;
	}

	ImDrawVert* vertices = (ImDrawVert*) mBufferVertex->GetMappedData();
	ImDrawIdx* indices = (ImDrawIdx*) mBufferIndex->GetMappedData();

	for ( s32 i = 0; i < drawData->CmdListsCount; i++ ) {
		const ImDrawList* jobs = drawData->CmdLists[i];

		size_t strideVertices = jobs->VtxBuffer.Size;
		size_t strideIndices = jobs->IdxBuffer.Size;

		memcpy( vertices, jobs->VtxBuffer.Data, strideVertices * sizeof( ImDrawVert ) );
		memcpy( indices, jobs->IdxBuffer.Data, strideIndices * sizeof( ImDrawIdx ) );

		vertices += strideVertices;
		indices += strideIndices;
	}

	mBufferVertex->Flush();
	mBufferIndex->Flush();

	VkCommandBuffer commandBuffer = mContext->GetCurrentCommandBuffer();

	s32 indexOffset = 0;
	s32 vertexOffset = 0;

	array<VkClearValue> clearValues( 2 );
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	ImGuiIO& io = ImGui::GetIO();
	ImVec2 displaySize = io.DisplaySize;

	VkPipelineLayout pipelineLayout = mUniformLayout->GetPipelineLayout();
	VkDescriptorSet descriptorSet = mUniformLayout->GetDescriptorSet();

	VkBuffer vertexBuffer = mBufferVertex->GetAPIHandle();

	vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderState->GetPipeline() );
	vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr );

	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers( commandBuffer, 0, 1, &vertexBuffer, offsets );
	vkCmdBindIndexBuffer( commandBuffer, mBufferIndex->GetAPIHandle(), 0, VK_INDEX_TYPE_UINT16 );

	VkViewport viewport = { 0.0f, 0.0f, displaySize.x, displaySize.y, 0.0f, 1.0f };
	vkCmdSetViewport( commandBuffer, 0, 1, &viewport );

	mPushConstantBlock.mPosition = glm::vec2( -1.0f );
	mPushConstantBlock.mScale = glm::vec2( 2.0f / displaySize.x, 2.0f / displaySize.y );
	vkCmdPushConstants( commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof( pushConstantBlock_t ), &mPushConstantBlock );

	for ( s32 drawListIndex = 0; drawListIndex < drawData->CmdListsCount; drawListIndex++ ) {
		ImDrawList* drawList = drawData->CmdLists[drawListIndex];

		for ( s32 renderJobIndex = 0; renderJobIndex < drawList->CmdBuffer.size(); renderJobIndex++ ) {
			ImDrawCmd& renderJob = drawList->CmdBuffer[renderJobIndex];
			u32 numIndices = renderJob.ElemCount;

			ImVec4& clipRect = renderJob.ClipRect;

			VkRect2D scissorRect = {};
			scissorRect.offset = { static_cast<s32>( clipRect.x ), static_cast<s32>( clipRect.y ) };
			scissorRect.extent = { static_cast<u32>( clipRect.z - clipRect.x ), static_cast<u32>( clipRect.w - clipRect.y ) };
			vkCmdSetScissor( commandBuffer, 0, 1, &scissorRect );

			vkCmdDrawIndexed( commandBuffer, numIndices, 1, indexOffset, vertexOffset, 0 );

			indexOffset += numIndices;
		}

		vertexOffset += drawList->VtxBuffer.Size;
	}
}