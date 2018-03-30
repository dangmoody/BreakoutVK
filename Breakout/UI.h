#ifndef __UI_H__
#define __UI_H__

#include <mstd/mstd.h>

#include <vulkan/vulkan.h>

#pragma warning( disable : 4201 )
#include <glm/glm.hpp>
#pragma warning( default : 4201 )

#include <imgui/imgui.h>

class VulkanContext;

class Buffer;
class Shader;
class Texture;

class UniformLayout;
class RenderState;

/*
================================================================================================

	Breakout UI

	Only renders text. Handles text input when entering a high score and displays high scores.

	Rendering code from the Imgui Vulkan example.

================================================================================================
*/

class UI {
public:
							UI();
	virtual					~UI();

	void					Init( const u32 screenWidth, const u32 screenHeight );
	void					Shutdown();
	bool32					IsInitialised() const { return mInitialised; }

	void					PushWindow( const ImVec2& pos, const ImVec4& backgroundColor );
	void					PopWindow();

	void					Begin();
	void					End();

	void					Render();

private:
	struct pushConstantBlock_t {
		glm::vec2			mPosition;
		glm::vec2			mScale;
	};

	pushConstantBlock_t		mPushConstantBlock;

	VulkanContext*			mContext;

	Texture*				mFontTexture;

	Buffer*					mBufferVertex;
	Buffer*					mBufferIndex;

	Shader*					mShaderVertex;
	Shader*					mShaderFragment;

	RenderState*			mRenderState;
	UniformLayout*			mUniformLayout;

	ImVec4					mBackgroundColor;
	ImVec2					mIMPos;

	VkDescriptorPool		mDescriptorPool;

	size_t					mBufferSizeVertex, mBufferSizeIndex;

	u32						mNumVertices, mNumIndices;

	u32						mWidth, mHeight;

	u32						mWindowCounter;

	bool32					mInitialised;
};

extern UI* gUI;

#endif // __UI_H__