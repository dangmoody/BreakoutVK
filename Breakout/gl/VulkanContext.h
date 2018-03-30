#ifndef __VULKAN_CONTEXT_H__
#define __VULKAN_CONTEXT_H__

#include <mstd/mstd.h>

#if MSTD_OS_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR		1
#endif

#include <vulkan/vulkan.h>
#include "vma/vma.h"

#include "Buffer.h"
#include "Texture.h"
#include "UniformLayout.h"
#include "RenderState.h"

// these conversion constants aren't supposed to live here but idk where the correct place is yet
#define GB_TO_MB						( 1024 )
#define GB_TO_KB						( 1024 * 1024 )
#define GB_TO_BYTES						( 1024 * 1024 * 1024 )

#define MB_TO_GB						( 1.0f / GB_TO_MB )
#define MB_TO_KB						( 1024 )
#define MB_TO_BYTES						( 1024 * 1024 )

#define KB_TO_GB						( 1.0f / GB_TO_KB )
#define KB_TO_MB						( 1.0f * 1024.0f )
#define KB_TO_BYTES						( 1024 )

#define BYTES_TO_GB						( 1.0f / GB_TO_BYTES )
#define BYTES_TO_MB						( 1.0f / MB_TO_BYTES )
#define BYTES_TO_KB						( 1.0f / KB_TO_BYTES )

// debug/release mode based defines
#if MSTD_DEBUG
#define YETI_VK_CHECK( x )				VulkanContext::CheckResult( x )
#define YETI_VK_STANDARD_VALIDATION		1
#else
#define YETI_VK_CHECK( x )				( x )
#define YETI_VK_STANDARD_VALIDATION		0
#endif

#define YETI_DEFAULT_BYTE_ALIGNMENT		16

#define YETI_VK_GET_INSTANCE_PROC_ADDRESS( instance, procAddress )													\
	fp##procAddress = reinterpret_cast<PFN_vk##procAddress>( vkGetInstanceProcAddr( instance, "vk"#procAddress ) );	\
	if ( fp##procAddress == NULL ) {																				\
		fatalError( "Vulkan instance function pointer \"%s\" could not be loaded!\n", "vk"#procAddress );			\
	}

#define YETI_VK_GET_DEVICE_PROC_ADDRESS( device, procAddress )														\
	fp##procAddress = reinterpret_cast<PFN_vk##procAddress>( vkGetDeviceProcAddr( device, "vk"#procAddress ) );		\
	if ( fp##procAddress == NULL ) {																				\
		fatalError( "Vulkan device function pointer \"%s\" could not be loaded!\n", "vk"#procAddress );				\
	}

#define YETI_FREE( x )					if ( ( x ) ) { delete ( x ); ( x ) = nullptr; }
#define YETI_FREE_ARRAY( x )			if ( ( x ) ) { delete [] ( x ); ( x ) = nullptr; }
#define YETI_FREE_ALIGNED( x )			if ( ( x ) ) { _aligned_free( ( x ) ); ( x ) = nullptr; }

class StagingManager;

// TODO: packing
struct gpuInfo_t {
	array<VkExtensionProperties>		mDeviceExtensionProperties;
	array<VkQueueFamilyProperties>		mQueueFamilyProperties;
	array<VkSurfaceFormatKHR>			mWindowSurfaceFormats;
	array<VkPresentModeKHR>				mPresentModes;

	VkPhysicalDevice					mGPUHandle;
	VkPhysicalDeviceProperties			mProperties;
	VkPhysicalDeviceMemoryProperties	mMemoryProperties;
	VkPhysicalDeviceFeatures			mFeatures;

	VkSurfaceCapabilitiesKHR			mWindowSurfaceCapabilities;
};

enum queueType_t {
	YETI_QUEUE_TYPE_GRAPHICS			= 0,
	YETI_QUEUE_TYPE_PRESENT,

	YETI_QUEUE_TYPE_COUNT
};

struct contextInitInfo_t {
	string								mApplicationName;

	u32									mWidth, mHeight, mNumBuffers;

	// TODO: macOS, linux
#if MSTD_OS_WINDOWS
	HINSTANCE							mHInstance;
	HWND								mHwnd;
#endif
};

/*
================================================================================================

	Yeti Vulkan Context

	Responsible for initialising a basic Vulkan Context and handling graphics API specific
	things. Also responsible for Swap-chain behaviour and data.

================================================================================================
*/

class VulkanContext {
public:
										VulkanContext();
	virtual								~VulkanContext();

	void								Init( const contextInitInfo_t& initInfo );
	void								Shutdown();
	inline bool32						IsInitialised() const { return mInitialised; }

	inline const gpuInfo_t&				GetActiveGPU() const { return mActiveGPU; }

	inline StagingManager*				GetStagingManager() { return mStagingManager; }

	inline VkDevice						GetLogicalDevice() const { return mLogicalDevice; }

	inline VmaAllocator					GetAllocator() const { return mAllocator; }

	inline VkRenderPass					GetRenderPass() const { return mRenderPass; }

	inline VkCommandBuffer				GetCurrentCommandBuffer() const { return mCommandBuffers[mCurrentImageIndex]; }

	inline VkQueue						GetQueue( const queueType_t queueType ) const { return mQueues[queueType]; }
	inline u32							GetQueueIndex( const queueType_t queueType ) const { return mQueueFamilyIndices[queueType]; }

	inline u32							GetMemoryTypeIndex( const VmaMemoryUsage memoryFlags, const VkMemoryRequirements& memoryRequirements ) const;

	void								Clear();
	void								Present();

	void								Resize( const u32 width, const u32 height );

	void								WaitDeviceIdle() const;

	// returns -1 if invalid format is passed
	inline s32							GetBitsPerPixelFromFormat( const VkFormat format ) const;

#if MSTD_DEBUG
	// checks if any errors get returned from a vulkan VkResult call
	static inline VkResult				CheckResult( const VkResult& result );
#endif

private:
	array<VkImage>						mSwapChainImages;
	array<VkImageView>					mSwapChainImageViews;

	array<VkFramebuffer>				mFramebuffers;

	array<VkCommandBuffer>				mCommandBuffers;

	array<VkFence>						mFences;

	array<bool32>						mCommandBuffersSubmitted;

	gpuInfo_t							mActiveGPU;

	StagingManager*						mStagingManager;

	VkInstance							mInstance;
	VkDevice							mLogicalDevice;

	VmaAllocator						mAllocator;

	VkSurfaceFormatKHR					mSurfaceFormat;

	VkSwapchainKHR						mSwapChain;

	VkSurfaceKHR						mWindowSurface;

	VkRenderPass						mRenderPass;

	VkCommandPool						mCommandPool;

	VkSemaphore							mSemaphoreAcquireImage, mSemaphoreRenderComplete;

	VkQueryPool							mQueryPoolTimestamps;
	float64								mFrameDelta;
	const u32							NUM_GPU_QUERIES = 2;

#if MSTD_DEBUG
	VkDebugReportCallbackEXT			mDebugReportCallback;
	VkDebugReportCallbackCreateInfoEXT	mDebugReportCallbackInfo;

	PFN_vkCreateDebugReportCallbackEXT	fpCreateDebugReportCallbackEXT	= VK_NULL_HANDLE;
	PFN_vkDestroyDebugReportCallbackEXT	fpDestroyDebugReportCallbackEXT	= VK_NULL_HANDLE;
#endif

	VkQueue								mQueues[YETI_QUEUE_TYPE_COUNT];
	u32									mQueueFamilyIndices[YETI_QUEUE_TYPE_COUNT];

	u32									mWidth, mHeight, mNumBuffers;
	u32									mCurrentImageIndex;

	bool32								mInitialised;

private:
	void								CreateInstance( const contextInitInfo_t& initInfo );
	void								DestroyInstance();

	void								CreateDebugLayer();
	void								DestroyDebugLayer();

	void								CreateSurface( const contextInitInfo_t& initInfo );
	void								DestroySurface();

	void								SelectPhysicalDevice();

	void								CreateLogicalDevice();
	void								DestroyLogicalDevice();

	void								CreateAllocator();
	void								DestroyAllocator();

	void								CreateSwapChain();
	void								DestroySwapChain();
	void								RecreateSwapChain();

	void								CreateRenderPass();
	void								DestroyRenderPass();

	void								CreateFramebuffers();
	void								DestroyFramebuffers();

	void								CreateCommandBuffers();
	void								DestroyCommandBuffers();

	void								CreateSemaphores();
	void								DestroySemaphores();

	void								CreateFences();
	void								DestroyFences();

	void								CreateQueryPool();
	void								DestroyQueryPool();
};

/*
========================
VulkanContext::GetMemoryTypeIndex
========================
*/
u32 VulkanContext::GetMemoryTypeIndex( const VmaMemoryUsage memoryFlags, const VkMemoryRequirements& memoryRequirements ) const {
	VkMemoryPropertyFlags desiredFlags = 0;

	switch ( memoryFlags ) {
	case VMA_MEMORY_USAGE_CPU_ONLY:
		desiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		break;

	case VMA_MEMORY_USAGE_CPU_TO_GPU:
		desiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		break;

	case VMA_MEMORY_USAGE_GPU_TO_CPU:
		desiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		break;

	case VMA_MEMORY_USAGE_GPU_ONLY:
		desiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		break;

	default:
		fatalError( "Bad memory usage flags: 0x%X.\n", memoryFlags );
		return 0;
	}

	u32 memoryTypeBits = memoryRequirements.memoryTypeBits;

	for ( u32 i = 0; i < VK_MAX_MEMORY_TYPES; i++ ) {
		VkMemoryType memoryType = mActiveGPU.mMemoryProperties.memoryTypes[i];

		if ( ( ( memoryTypeBits >> i ) & 1 ) == 0 ) {
			continue;
		}

		if ( ( memoryType.propertyFlags & desiredFlags ) == desiredFlags ) {
			return i;
		}
	}

	error( "Unable to find a memoryTypeIndex for the VkMemoryAllocateInfo.\n" );

	return 0;
}

/*
========================
VulkanContext::GetBitsPerPixelFromFormat
========================
*/
s32 VulkanContext::GetBitsPerPixelFromFormat( const VkFormat format ) const {
	// TODO: add more texture formats when needed
	switch ( format ) {
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
			return 32;

		default:
			error( "GetBitsPerPixelFromFormat() failed because an invalid/unsupported VkFormat was passed!\n" );
			return -1;
	}
}

/*
========================
VulkanContext::CheckResult
========================
*/
#if MSTD_DEBUG
VkResult VulkanContext::CheckResult( const VkResult& result ) {
	if ( result < 0 ) {
		char* msg = nullptr;
		switch ( result ) {
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			msg = "VK_ERROR_OUT_OF_HOST_MEMORY";
			break;

		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			msg = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			break;

		case VK_ERROR_INITIALIZATION_FAILED:
			msg = "VK_ERROR_INITIALIZATION_FAILED";
			break;

		case VK_ERROR_DEVICE_LOST:
			msg = "VK_ERROR_DEVICE_LOST";
			break;

		case VK_ERROR_MEMORY_MAP_FAILED:
			msg = "VK_ERROR_MEMORY_MAP_FAILED";
			break;

		case VK_ERROR_LAYER_NOT_PRESENT:
			msg = "VK_ERROR_LAYER_NOT_PRESENT";
			break;

		case VK_ERROR_EXTENSION_NOT_PRESENT:
			msg = "VK_ERROR_EXTENSION_NOT_PRESENT";
			break;

		case VK_ERROR_FEATURE_NOT_PRESENT:
			msg = "VK_ERROR_FEATURE_NOT_PRESENT";
			break;

		case VK_ERROR_INCOMPATIBLE_DRIVER:
			msg = "VK_ERROR_INCOMPATIBLE_DRIVER";
			break;

		case VK_ERROR_TOO_MANY_OBJECTS:
			msg = "VK_ERROR_TOO_MANY_OBJECTS";
			break;

		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			msg = "VK_ERROR_FORMAT_NOT_SUPPORTED";
			break;

		case VK_ERROR_SURFACE_LOST_KHR:
			msg = "VK_ERROR_SURFACE_LOST_KHR";
			break;

		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			msg = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			break;

		case VK_SUBOPTIMAL_KHR:
			msg = "VK_SUBOPTIMAL_KHR";
			break;

		case VK_ERROR_OUT_OF_DATE_KHR:
			msg = "VK_ERROR_OUT_OF_DATE_KHR";
			break;

		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			msg = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
			break;

		case VK_ERROR_VALIDATION_FAILED_EXT:
			msg = "VK_ERROR_VALIDATION_FAILED_EXT";
			break;

		case VK_TIMEOUT:
			msg = "VK_TIMEOUT";
			break;

		default:
			msg = "UNKNOWN_ERROR";
			break;
		}

		error( "%s\n", msg );
	}

	return result;
}
#endif

#endif // __VULKAN_CONTEXT_H__