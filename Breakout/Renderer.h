#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <mstd/mstd.h>

#include <glm/glm.hpp>

#include "gl/gl_main.h"

struct vertex_t {
	glm::vec3							mPos;
};

struct uniformDataStatic_t {
	glm::mat4							mViewProjection;
};

struct uniformDataQuad_t {
	glm::mat4							mModel;
	glm::vec4							mColor;
	glm::vec2							mScale;
};

/*
================================================================================================

	Breakout Renderer

	Built to render quads only. Contains all the mesh data for a quad due there only being a
	need for one quad mesh. Also responsible for initialising camera.

================================================================================================
*/

class Renderer {
public:
	// corrects vulkan's upside-down clip space
	static const glm::mat4				CLIP_MATRIX;

public:
										Renderer();
	virtual								~Renderer();

	void								Init();
	void								Shutdown();
	inline bool32						IsInitialised() const { return mInitialised; }

	inline VulkanContext*				GetContext() { return mContext; }
	inline const VulkanContext*			GetContext() const { return mContext; }

	inline glm::mat4					GetWorldToClip() const { return mMatrixProjection * mMatrixView; }
	inline glm::mat4					GetClipToWorld() const { return glm::inverse( GetWorldToClip() ); }

	void								Resize( const u32 width, const u32 height );

	void								AddRenderJob( const uniformDataQuad_t& jobData );

	void								StartFrame();
	void								EndFrame();
	void								DrawElements();

private:
	VulkanContext*						mContext;

	Buffer*								mBufferVertex;
	Buffer*								mBufferIndex;
	Buffer*								mBufferUniformStatic;
	Buffer*								mBufferUniformQuad;

	Shader*								mShaderVertex;
	Shader*								mShaderFragment;

	RenderState*						mRenderState;
	UniformLayout*						mUniformLayout;

	array<uniformDataQuad_t>			mJobs;
	uniformDataStatic_t					mUniformDataStatic;

	array<vertex_t>						mVertices;
	array<u32>							mIndices;

	glm::mat4							mMatrixView, mMatrixProjection;

	VkDescriptorPool					mDescriptorPool;

	size_t								mDynamicAlignment;

	float32								mAspectRatio;

	bool32								mInitialised;

private:
	void								CreateBuffers();
	void								DestroyBuffers();

	void								CreateShaders();
	void								DestroyShaders();

	void								CreateRenderState();
	void								DestroyRenderState();
};

extern Renderer* gRenderer;

#endif // __RENDERER_H__