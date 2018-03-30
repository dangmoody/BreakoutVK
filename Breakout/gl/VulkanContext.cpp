#include "gl_main.h"

#include "../Window.h"

/*
================================================================================================

	VulkanContext

================================================================================================
*/

/*
========================
VulkanContext::VulkanContext
========================
*/
VulkanContext::VulkanContext() {
	mSwapChain = nullptr;
	mStagingManager = nullptr;

	mStagingManager = nullptr;

	mSurfaceFormat = {};

	mSwapChain = VK_NULL_HANDLE;

	mWindowSurface = VK_NULL_HANDLE;

	mRenderPass = VK_NULL_HANDLE;

	mCommandPool = VK_NULL_HANDLE;

	mSemaphoreAcquireImage = VK_NULL_HANDLE;
	mSemaphoreRenderComplete = VK_NULL_HANDLE;

	mWindowSurface = VK_NULL_HANDLE;

#if MSTD_DEBUG
	mDebugReportCallback = VK_NULL_HANDLE;
	mDebugReportCallbackInfo = {};
#endif

	mWidth = mHeight = 0;
	mNumBuffers = 0;

	mInitialised = false;
}

/*
========================
VulkanContext::~VulkanContext
========================
*/
VulkanContext::~VulkanContext() {
	Shutdown();
}

/*
========================
VulkanContext::Clear
========================
*/
void VulkanContext::Clear() {
	mStagingManager->Flush();

	VkResult result = vkAcquireNextImageKHR( mLogicalDevice, mSwapChain, U64_MAX, mSemaphoreAcquireImage, VK_NULL_HANDLE, &mCurrentImageIndex );
	if ( result == VK_ERROR_OUT_OF_DATE_KHR ) {
		RecreateSwapChain();
	} else {
		YETI_VK_CHECK( result );
	}

	VkRect2D renderArea = {};
	renderArea.offset = { 0, 0 };
	renderArea.extent = { mWidth, mHeight };

	VkCommandBuffer& currentCommandBuffer = mCommandBuffers[mCurrentImageIndex];

	VkFence* fence = &mFences[mCurrentImageIndex];
	if ( mCommandBuffersSubmitted[mCurrentImageIndex] ) {
		YETI_VK_CHECK( vkWaitForFences( mLogicalDevice, 1, fence, VK_TRUE, U64_MAX ) );
	}

	YETI_VK_CHECK( vkResetFences( mLogicalDevice, 1, fence ) );

	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	YETI_VK_CHECK( vkResetCommandBuffer( currentCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT ) );
	YETI_VK_CHECK( vkBeginCommandBuffer( currentCommandBuffer, &commandBufferBeginInfo ) );

	vkCmdResetQueryPool( currentCommandBuffer, mQueryPoolTimestamps, 0, NUM_GPU_QUERIES );

	VkClearValue clearValue = {};
	clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pClearValues = &clearValue;
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.framebuffer = mFramebuffers[mCurrentImageIndex];
	renderPassBeginInfo.renderArea = renderArea;
	renderPassBeginInfo.renderPass = mRenderPass;
	vkCmdBeginRenderPass( currentCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float32>( mWidth );
	viewport.height = static_cast<float32>( mHeight );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport( currentCommandBuffer, 0, 1, &viewport );

	vkCmdSetScissor( currentCommandBuffer, 0, 1, &renderArea );
}

/*
========================
VulkanContext::Present
========================
*/
void VulkanContext::Present() {
	VkCommandBuffer& currentCommandBuffer = mCommandBuffers[mCurrentImageIndex];

	vkCmdWriteTimestamp( currentCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, mQueryPoolTimestamps, 0 );
	vkCmdWriteTimestamp( currentCommandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, mQueryPoolTimestamps, 1 );

	vkCmdEndRenderPass( currentCommandBuffer );
	YETI_VK_CHECK( vkEndCommandBuffer( currentCommandBuffer ) );

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &currentCommandBuffer;
	submitInfo.commandBufferCount = 1;
	submitInfo.pSignalSemaphores = &mSemaphoreRenderComplete;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &mSemaphoreAcquireImage;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = &waitStage;
	YETI_VK_CHECK( vkQueueSubmit( mQueues[YETI_QUEUE_TYPE_GRAPHICS], 1, &submitInfo, mFences[mCurrentImageIndex] ) );

	{
		u32 stride = sizeof( u64 );
		VkQueryResultFlags queryFlags = VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT;
		float64 factor = 1000000.0 * mActiveGPU.mProperties.limits.timestampPeriod;

		u64 start = 0;
		u64 end = 0;
		YETI_VK_CHECK( vkGetQueryPoolResults( mLogicalDevice, mQueryPoolTimestamps, 0, 1, stride, &start, 0, queryFlags ) );
		YETI_VK_CHECK( vkGetQueryPoolResults( mLogicalDevice, mQueryPoolTimestamps, 1, 1, stride, &end, 0, queryFlags ) );

		mFrameDelta = ( end - start ) / factor;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pImageIndices = &mCurrentImageIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &mSwapChain;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &mSemaphoreRenderComplete;
	VkResult result = vkQueuePresentKHR( mQueues[YETI_QUEUE_TYPE_PRESENT], &presentInfo );
	if ( result == VK_ERROR_OUT_OF_DATE_KHR ) {
		RecreateSwapChain();
	} else {
		YETI_VK_CHECK( result );
	}

	mCommandBuffersSubmitted[mCurrentImageIndex] = true;
}

/*
========================
VulkanContext::Resize
========================
*/
void VulkanContext::Resize( const u32 width, const u32 height ) {
	assertf( width > 0, "Specified resize width was 0!" );
	assertf( height > 0, "Specified resize height was 0!" );

	mWidth = width;
	mHeight = height;

	RecreateSwapChain();
}

/*
========================
VulkanContext::WaitDeviceIdle
========================
*/
void VulkanContext::WaitDeviceIdle() const {
	YETI_VK_CHECK( vkDeviceWaitIdle( mLogicalDevice ) );
}