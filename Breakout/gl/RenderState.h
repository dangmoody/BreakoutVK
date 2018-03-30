#ifndef __RENDER_STATE_H__
#define __RENDER_STATE_H__

class Shader;
class UniformLayout;

/*
================================================================================================

	Yeti Render State

	Combination of Depth-Stencil, Raster, and Blend states. Also keeps track of its shaders and
	vertex topology. Gets created via a renderStateDesc_t.

================================================================================================
*/

// TODO: source/dest blending options and blend op
struct renderStateDesc_t {
	Shader*								mVertexShader;
	Shader*								mFragmentShader;

	UniformLayout*						mUniformLayout;
	
	VkVertexInputBindingDescription*	mVertexBindings;
	VkVertexInputAttributeDescription*	mVertexAttribs;

	u32									mNumVertexBindings;
	u32									mNumVertexAttribs;

	VkCompareOp							mDepthTestOp;
	bool32								mEnableDepthTest;
	bool32								mEnableDepthWrite;
	bool32								mEnableStencil;
	bool32								mEnableAlpha;

	VkCullModeFlagBits					mCullMode;
	VkFrontFace							mFrontFace;
	float32								mLineWidth;
	VkPolygonMode						mPolygonMode;

	VkPrimitiveTopology					mTopology;
};

class RenderState {
public:
										RenderState( VulkanContext* context );
										~RenderState();

	inline bool32						IsAlloced() const { return mPipeline != VK_NULL_HANDLE; }

	void								AllocRenderState( const renderStateDesc_t& desc );
	void								UnallocRenderState();

	VkPipeline							GetPipeline() const { return mPipeline; }

private:
	VulkanContext*						mContext;

	VkPipeline							mPipeline;
};

#endif // __RENDER_STATE_H__