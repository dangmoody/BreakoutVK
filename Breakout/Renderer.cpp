#pragma warning( disable : 4201 )
#include <glm/gtc/matrix_transform.hpp>
#pragma warning( default : 4201 )

#include "Renderer.h"

#include "Defines.h"

#include "Window.h"

/*
================================================================================================

	Renderer

================================================================================================
*/

Renderer* gRenderer = nullptr;

const glm::mat4 Renderer::CLIP_MATRIX = glm::mat4( 1.0f, 0.0f, 0.0f, 0.0f,
												0.0f, -1.0f, 0.0f, 0.0f,
												0.0f, 0.0f, 0.5f, 0.0f,
												0.0f, 0.0f, 0.5f, 1.0f );

/*
========================
Renderer::Renderer
========================
*/
Renderer::Renderer() {
	mContext = nullptr;

	mBufferVertex = nullptr;
	mBufferIndex = nullptr;

	mShaderVertex = nullptr;
	mShaderFragment = nullptr;

	mRenderState = nullptr;
	mUniformLayout = nullptr;

	mUniformDataStatic = {};

	mDescriptorPool = VK_NULL_HANDLE;

	mAspectRatio = 0.0f;

	mDynamicAlignment = 0;

	mInitialised = false;
}

/*
========================
Renderer::~Renderer
========================
*/
Renderer::~Renderer() {
	Shutdown();
}

/*
========================
Renderer::Init
========================
*/
void Renderer::Init() {
	if ( IsInitialised() ) {
		return;
	}

	printf( "------- Initialising Renderer -------\n" );

	mAspectRatio = static_cast<float32>( GAME_WIDTH ) / static_cast<float32>( GAME_HEIGHT );

	mContext = new VulkanContext();

	contextInitInfo_t initInfo = {};
	initInfo.mApplicationName = GAME_NAME;
	initInfo.mWidth = GAME_WIDTH;
	initInfo.mHeight = GAME_HEIGHT;
	initInfo.mNumBuffers = RENDERER_NUM_BUFFERS;
#if MSTD_OS_WINDOWS
	initInfo.mHInstance = gWindow->GetHInstance();
	initInfo.mHwnd = gWindow->GetHwnd();
#endif

	mContext->Init( initInfo );

	// calculate dynamic uniform buffer alignment stride
	size_t alignMod = static_cast<size_t>( mContext->GetActiveGPU().mProperties.limits.minUniformBufferOffsetAlignment );
	mDynamicAlignment = ( sizeof( uniformDataQuad_t ) / alignMod ) * alignMod;
	if ( sizeof( uniformDataQuad_t ) % alignMod > 0 ) {
		mDynamicAlignment += alignMod;
	}

	mJobs.reserve( NUM_BLOCKS_MAX );

	mVertices = {
		{ glm::vec3( -1.0f, 1.0f, 0.0f ) },
		{ glm::vec3( 1.0f, 1.0f, 0.0f ) },
		{ glm::vec3( 1.0f, -1.0f, 0.0f ) },
		{ glm::vec3( -1.0f, -1.0f, 0.0f ) },
	};

	mIndices = {
		0, 1, 2,
		0, 2, 3
	};

	CreateBuffers();

	CreateShaders();

	CreateRenderState();

	// init camera
	{
		mMatrixView = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, -1.0f ) );

		float32 left = -mAspectRatio * ORTHO_SIZE;
		float32 right = mAspectRatio * ORTHO_SIZE;
		float32 top = -ORTHO_SIZE;
		float32 bottom = ORTHO_SIZE;
		mMatrixProjection = glm::ortho( left, right, top, bottom, -1.0f, 100.0f );

		mUniformDataStatic.mViewProjection = CLIP_MATRIX * mMatrixProjection * mMatrixView;
		mBufferUniformStatic->UploadData( &mUniformDataStatic, sizeof( uniformDataStatic_t ) );
	}

	mInitialised = true;

	printf( "------- Renderer initialised -------\n\n" );
}

/*
========================
Renderer::Shutdown
========================
*/
void Renderer::Shutdown() {
	if ( !IsInitialised() ) {
		return;
	}

	vkDestroyDescriptorPool( mContext->GetLogicalDevice(), mDescriptorPool, nullptr );
	mDescriptorPool = VK_NULL_HANDLE;

	DestroyRenderState();

	DestroyShaders();

	DestroyBuffers();

	YETI_FREE( mContext );

	mInitialised = false;
}

/*
========================
Renderer::Resize
========================
*/
void Renderer::Resize( const u32 width, const u32 height ) {
	assertf( width > 0, "Specified resize width was 0!" );
	assertf( height > 0, "Specified resize height was 0!" );

	DestroyRenderState();

	mContext->Resize( width, height );

	CreateRenderState();
}

/*
========================
Renderer::AddRenderJob
========================
*/
void Renderer::AddRenderJob( const uniformDataQuad_t& jobData ) {
	mJobs.add( jobData );
}

/*
========================
Renderer::StartFrame
========================
*/
void Renderer::StartFrame() {
	mJobs.clear();

	mContext->Clear();
}

/*
========================
Renderer::EndFrame
========================
*/
void Renderer::EndFrame() {
	mContext->Present();
}

/*
========================
Renderer::DrawElements
========================
*/
void Renderer::DrawElements() {
	size_t numJobs = mJobs.length();

	if ( numJobs == 0 ) {
		return;
	}

	// copy all uniform data from jobs into the dynamic uniform buffer
	u8* mappedUniform = reinterpret_cast<u8*>( mBufferUniformQuad->GetMappedData() );
	for ( size_t i = 0; i < numJobs; i++ ) {
		memcpy( &mappedUniform[i * mDynamicAlignment], &mJobs[i], sizeof( uniformDataQuad_t ) );
	}

	VkDeviceSize offsetsVertex[1] = { 0 };

	VkCommandBuffer commandBuffer = mContext->GetCurrentCommandBuffer();

	VkBuffer vertexBuffer = mBufferVertex->GetAPIHandle();
	VkDescriptorSet descriptorSet = mUniformLayout->GetDescriptorSet();

	// don't re-bind vertex/index buffers and render states
	// because we know all surfaces in this game are using the same render state data everywhere
	vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderState->GetPipeline() );
	vkCmdBindVertexBuffers( commandBuffer, 0, 1, &vertexBuffer, offsetsVertex );
	vkCmdBindIndexBuffer( commandBuffer, mBufferIndex->GetAPIHandle(), 0, VK_INDEX_TYPE_UINT32 );

	for ( size_t i = 0; i < numJobs; i++ ) {
		u32 uniformDataQuadOffset = static_cast<u32>( i * mDynamicAlignment );

		vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mUniformLayout->GetPipelineLayout(), 0, 1, &descriptorSet, 1, &uniformDataQuadOffset );
		vkCmdDrawIndexed( commandBuffer, static_cast<u32>( mIndices.length() ), 1, 0, 0, 0 );
	}
}

/*
========================
Renderer::CreateBuffers
========================
*/
void Renderer::CreateBuffers() {
	size_t bufferSizeVertex = mVertices.length() * sizeof( vertex_t );
	size_t bufferSizeIndex = mIndices.length() * sizeof( u32 );
	size_t bufferSizeUniformStatic = sizeof( uniformDataStatic_t );
	size_t bufferSizeUniformQuad = ( NUM_BLOCKS_MAX + 2 ) * mDynamicAlignment;	// + 2 for paddle and ball

	bufferDesc_t bufferDescVertex = {};
	bufferDescVertex.mBufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferDescVertex.mMemoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	bufferDescVertex.mData = mVertices.data();
	bufferDescVertex.mDataSizeBytes = bufferSizeVertex;
	mBufferVertex = new Buffer( mContext );
	mBufferVertex->AllocBuffer( bufferDescVertex );

	bufferDesc_t bufferDescIndex = {};
	bufferDescIndex.mBufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferDescIndex.mMemoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	bufferDescIndex.mData = mIndices.data();
	bufferDescIndex.mDataSizeBytes = bufferSizeIndex;
	mBufferIndex = new Buffer( mContext );
	mBufferIndex->AllocBuffer( bufferDescIndex );

	bufferDesc_t bufferDescUniformStatic = {};
	bufferDescUniformStatic.mBufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferDescUniformStatic.mMemoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	bufferDescUniformStatic.mData = &mUniformDataStatic;
	bufferDescUniformStatic.mDataSizeBytes = bufferSizeUniformStatic;
	mBufferUniformStatic = new Buffer( mContext );
	mBufferUniformStatic->AllocBuffer( bufferDescUniformStatic );

	bufferDesc_t bufferDescUniformQuad = {};
	bufferDescUniformQuad.mBufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferDescUniformQuad.mMemoryUsage = VMA_MEMORY_USAGE_CPU_ONLY;
	bufferDescUniformQuad.mData = mJobs.data();
	bufferDescUniformQuad.mDataSizeBytes = bufferSizeUniformQuad;
	mBufferUniformQuad = new Buffer( mContext );
	mBufferUniformQuad->AllocBuffer( bufferDescUniformQuad );
}

/*
========================
Renderer::DestroyBuffers
========================
*/
void Renderer::DestroyBuffers() {
	mBufferUniformQuad->UnallocBuffer();
	YETI_FREE( mBufferUniformQuad );

	mBufferUniformStatic->UnallocBuffer();
	YETI_FREE( mBufferUniformStatic );

	mBufferIndex->UnallocBuffer();
	YETI_FREE( mBufferIndex );

	mBufferVertex->UnallocBuffer();
	YETI_FREE( mBufferVertex );
}

/*
========================
Renderer::CreateShaders
========================
*/
void Renderer::CreateShaders() {
	shaderDesc_t shaderDesc = {};
	shaderDesc.mShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderDesc.mFilename = BASE_PATH "shader_binaries/unlit_3d.vert.spv";
	mShaderVertex = new Shader( mContext );
	mShaderVertex->AllocShader( shaderDesc );

	shaderDesc.mShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderDesc.mFilename = BASE_PATH "shader_binaries/unlit_3d.frag.spv";
	mShaderFragment = new Shader( mContext );
	mShaderFragment->AllocShader( shaderDesc );
}

/*
========================
Renderer::DestroyShaders
========================
*/
void Renderer::DestroyShaders() {
	mShaderFragment->UnallocShader();
	YETI_FREE( mShaderFragment );

	mShaderVertex->UnallocShader();
	YETI_FREE( mShaderVertex );
}

/*
========================
Renderer::CreateRenderState
========================
*/
void Renderer::CreateRenderState() {
	VkVertexInputBindingDescription vertexBinding = {};
	vertexBinding.binding = 0;
	vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexBinding.stride = sizeof( glm::vec3 );

	array<VkVertexInputAttributeDescription> vertexAttribs = {
		{ 0, vertexBinding.binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof( vertex_t, mPos ) },
	};

	array<VkDescriptorSetLayoutBinding> uniformBindings = {
		{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
		{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
	};

	array<Buffer*> uniformBuffers = {
		mBufferUniformStatic,
		mBufferUniformQuad,
	};

	array<VkDescriptorPoolSize> poolSizes = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
	};

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.maxSets = 1;
	descPoolInfo.poolSizeCount = static_cast<u32>( poolSizes.length() );
	descPoolInfo.pPoolSizes = poolSizes.data();
	YETI_VK_CHECK( vkCreateDescriptorPool( mContext->GetLogicalDevice(), &descPoolInfo, nullptr, &mDescriptorPool ) );

	uniformLayoutDesc_t uniformLayoutDesc = {};
	uniformLayoutDesc.mDescriptorPool = mDescriptorPool;
	uniformLayoutDesc.mBindings = uniformBindings.data();
	uniformLayoutDesc.mNumBindings = static_cast<u32>( uniformBindings.length() );
	uniformLayoutDesc.mUniformBuffers = uniformBuffers;
	uniformLayoutDesc.mPushConstants = nullptr;
	uniformLayoutDesc.mNumPushConstants = 0;
	mUniformLayout = new UniformLayout( mContext );
	mUniformLayout->AllocUniformLayout( uniformLayoutDesc );

	renderStateDesc_t renderStateQuad = {};
	renderStateQuad.mCullMode = VK_CULL_MODE_BACK_BIT;
	renderStateQuad.mDepthTestOp = VK_COMPARE_OP_NEVER;
	renderStateQuad.mEnableAlpha = true;
	renderStateQuad.mEnableDepthTest = false;
	renderStateQuad.mEnableDepthWrite = false;
	renderStateQuad.mEnableStencil = false;
	renderStateQuad.mFrontFace = VK_FRONT_FACE_CLOCKWISE;
	renderStateQuad.mLineWidth = 1.0f;
	renderStateQuad.mPolygonMode = VK_POLYGON_MODE_FILL;
	renderStateQuad.mTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	renderStateQuad.mUniformLayout = mUniformLayout;
	renderStateQuad.mVertexAttribs = vertexAttribs.data();
	renderStateQuad.mNumVertexAttribs = static_cast<u32>( vertexAttribs.length() );
	renderStateQuad.mVertexBindings = &vertexBinding;
	renderStateQuad.mNumVertexBindings = 1;
	renderStateQuad.mVertexShader = mShaderVertex;
	renderStateQuad.mFragmentShader = mShaderFragment;
	mRenderState = new RenderState( mContext );
	mRenderState->AllocRenderState( renderStateQuad );
}

/*
========================
Renderer::DestroyRenderState
========================
*/
void Renderer::DestroyRenderState() {
	mUniformLayout->UnallocUniformLayout();
	YETI_FREE( mUniformLayout );

	mRenderState->UnallocRenderState();
	YETI_FREE( mRenderState );
}