#include <mstd/mstd.h>

#include <assert.h>

#include "VulkanContext.h"
#include "StagingManager.h"

#include "../Window.h"

/*
========================
DebugCallback
========================
*/
#if MSTD_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugReportFlagsEXT reportFlags, VkDebugReportObjectTypeEXT objectType,
	u64 sourceObject, u32 location, s32 msgCode, const char* layerPrefix, const char* msg, void* data ) {
	UNUSED( objectType );
	UNUSED( sourceObject );
	UNUSED( location );
	UNUSED( msgCode );
	UNUSED( data );

	const char* msgFormat = "VK_LOG: %s: @[ %s ]: %s.\n";
	const char* errorFormat = "VK_LOG: @[ %s ]: %s.\n";

	char* reportPrefix = nullptr;
	switch ( reportFlags ) {
	case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
		reportPrefix = "INFORMATION";
		printf( msgFormat, reportPrefix, layerPrefix, msg );
		break;

	case VK_DEBUG_REPORT_WARNING_BIT_EXT:
		reportPrefix = "WARNING";
		warning( errorFormat, layerPrefix, msg );
		break;

	case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
		reportPrefix = "PERFORMANCE WARNING";
		warning( errorFormat, layerPrefix, msg );
		break;

	case VK_DEBUG_REPORT_ERROR_BIT_EXT:
		reportPrefix = "ERROR";
		error( errorFormat, layerPrefix, msg );
		break;

	case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
		reportPrefix = "DEBUG";
		printf( msgFormat, reportPrefix, layerPrefix, msg );
		break;

	default:
		break;
	}

	return false;
}
#endif

/*
========================
VulkanContext::Init
========================
*/
void VulkanContext::Init( const contextInitInfo_t& initInfo ) {
	if ( IsInitialised() ) {
		error( "VulkanContext has already been initialised! Nothing will happen this time!\n" );
		return;
	}

#if MSTD_DEBUG
	timestamp_t start, end;
	float64 delta;

	printf( "------- Initialising Vulkan Context -------\n" );

	start = timeNow();
#endif

	mWidth = initInfo.mWidth;
	mHeight = initInfo.mHeight;
	mNumBuffers = initInfo.mNumBuffers;

	CreateInstance( initInfo );

	CreateDebugLayer();

	CreateSurface( initInfo );

	SelectPhysicalDevice();

	CreateLogicalDevice();

	CreateAllocator();

	CreateSwapChain();

	CreateRenderPass();

	CreateFramebuffers();

	CreateCommandBuffers();

	CreateSemaphores();

	CreateFences();

	CreateQueryPool();

	mStagingManager = new StagingManager();
	mStagingManager->Init( this, mNumBuffers );

	mInitialised = true;

#if MSTD_DEBUG
	end = timeNow();
	delta = deltaMilliseconds( start, end );

	printf( "------- Vulkan Context Initialised. Time Taken: %f ms -------\n\n", delta );
#endif
}

/*
========================
VulkanContext::Shutdown
========================
*/
void VulkanContext::Shutdown() {
	if ( !IsInitialised() ) {
		error( "Attempt to call VulkanContext::Shutdown() was made when it's not initialized!\n" );
		return;
	}

#if MSTD_DEBUG
	printf( "------- Vulkan Context shutting down -------\n" );
#endif

	YETI_VK_CHECK( vkDeviceWaitIdle( mLogicalDevice ) );

	YETI_FREE( mStagingManager );

	DestroyQueryPool();

	DestroyFences();

	DestroySemaphores();

	DestroyCommandBuffers();

	DestroyFramebuffers();

	DestroySwapChain();

	DestroyRenderPass();

	DestroyAllocator();

	DestroyDebugLayer();

	DestroyLogicalDevice();

	DestroySurface();

	DestroyInstance();

	mInitialised = false;

#if MSTD_DEBUG
	printf( "------- Vulkan Context shutdown -------\n" );
#endif
}

/*
========================
VulkanContext::CreateInstance
========================
*/
void VulkanContext::CreateInstance( const contextInitInfo_t& initInfo ) {
	array<const char*> instanceLayers = {
#if YETI_VK_STANDARD_VALIDATION
		"VK_LAYER_LUNARG_standard_validation",
#endif
	};

	array<const char*> instanceExtensions = {
#if YETI_VK_STANDARD_VALIDATION
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif

#if MSTD_OS_WINDOWS
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

		VK_KHR_SURFACE_EXTENSION_NAME,
	};

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_VERSION( 1, 0, VK_HEADER_VERSION );
	appInfo.pApplicationName = initInfo.mApplicationName.c_str();
	appInfo.pEngineName = "yeti1";
	appInfo.engineVersion = 1;

	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = static_cast<u32>( instanceLayers.length() );
	instanceInfo.ppEnabledLayerNames = instanceLayers.data();
	instanceInfo.enabledExtensionCount = static_cast<u32>( instanceExtensions.length() );
	instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
	YETI_VK_CHECK( vkCreateInstance( &instanceInfo, nullptr, &mInstance ) );
}

/*
========================
VulkanContext::DestroyInstance
========================
*/
void VulkanContext::DestroyInstance() {
	vkDestroyInstance( mInstance, nullptr );
	mInstance = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::CreateDebugLayer
========================
*/
void VulkanContext::CreateDebugLayer() {
#if YETI_VK_STANDARD_VALIDATION
	mDebugReportCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	mDebugReportCallbackInfo.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>( DebugCallback );
	mDebugReportCallbackInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;

	// TODO: see if we can generate these programmatically?
	YETI_VK_GET_INSTANCE_PROC_ADDRESS( mInstance, CreateDebugReportCallbackEXT );
	YETI_VK_GET_INSTANCE_PROC_ADDRESS( mInstance, DestroyDebugReportCallbackEXT );

	fpCreateDebugReportCallbackEXT( mInstance, &mDebugReportCallbackInfo, nullptr, &mDebugReportCallback );
#endif
}

/*
========================
VulkanContext::DestroyDebugLayer
========================
*/
void VulkanContext::DestroyDebugLayer() {
#if YETI_VK_STANDARD_VALIDATION
	fpDestroyDebugReportCallbackEXT( mInstance, mDebugReportCallback, nullptr );
	mDebugReportCallback = VK_NULL_HANDLE;
#endif
}

/*
========================
VulkanContext::CreateSurface
========================
*/
void VulkanContext::CreateSurface( const contextInitInfo_t& initInfo ) {
	// TODO: mac OS, linux
#if MSTD_OS_WINDOWS
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = initInfo.mHInstance;
	createInfo.hwnd = initInfo.mHwnd;
	YETI_VK_CHECK( vkCreateWin32SurfaceKHR( mInstance, &createInfo, nullptr, &mWindowSurface ) );
#endif
}

/*
========================
VulkanContext::DestroySurface
========================
*/
void VulkanContext::DestroySurface() {
	vkDestroySurfaceKHR( mInstance, mWindowSurface, nullptr );
	mWindowSurface = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::SelectPhysicalDevice
========================
*/
void VulkanContext::SelectPhysicalDevice() {
	array<gpuInfo_t> gpus;

	// enumerate physical device
	{
		u32 numGPUs = 0;
		YETI_VK_CHECK( vkEnumeratePhysicalDevices( mInstance, &numGPUs, nullptr ) );
		assertf( numGPUs > 0, "VulkanContext::SelectPhysicalDevice() failed to find any device!\n" );
		array<VkPhysicalDevice> physicalDevices( numGPUs );
		YETI_VK_CHECK( vkEnumeratePhysicalDevices( mInstance, &numGPUs, physicalDevices.data() ) );

		gpus.resize( numGPUs );

		// get the device properties of every GPU on our system
		for ( u32 i = 0; i < numGPUs; i++ ) {
			gpuInfo_t& gpu = gpus[i];
			gpu.mGPUHandle = physicalDevices[i];

			// device extensions
			{
				u32 numDeviceExtensions = 0;
				YETI_VK_CHECK( vkEnumerateDeviceExtensionProperties( gpu.mGPUHandle, nullptr, &numDeviceExtensions, nullptr ) );
				assertf( numDeviceExtensions > 0, "VulkanContext::SelectPhysicalDevice() failed to find the extension properties of your device!\n" );
				gpu.mDeviceExtensionProperties.resize( numDeviceExtensions );
				YETI_VK_CHECK( vkEnumerateDeviceExtensionProperties( gpu.mGPUHandle, nullptr, &numDeviceExtensions, gpu.mDeviceExtensionProperties.data() ) );
			}

			YETI_VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( gpu.mGPUHandle, mWindowSurface, &gpu.mWindowSurfaceCapabilities ) );

			// surface formats
			{
				u32 numSurfaceFormats = 0;
				YETI_VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( gpu.mGPUHandle, mWindowSurface, &numSurfaceFormats, nullptr ) );
				assertf( numSurfaceFormats > 0, "VulkanContext::SelectPhysicalDevice() failed to find the surface formats of your GPU!\n" );
				gpu.mWindowSurfaceFormats.resize( numSurfaceFormats );
				YETI_VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( gpu.mGPUHandle, mWindowSurface, &numSurfaceFormats, gpu.mWindowSurfaceFormats.data() ) );
			}

			// present modes
			{
				u32 numPresentModes = 0;
				YETI_VK_CHECK( vkGetPhysicalDeviceSurfacePresentModesKHR( gpu.mGPUHandle, mWindowSurface, &numPresentModes, nullptr ) );
				assertf( numPresentModes > 0, "VulkanContext::SelectPhysicalDevice() failed to find the surface present modes of your GPU!\n" );
				gpu.mPresentModes.resize( numPresentModes );
				YETI_VK_CHECK( vkGetPhysicalDeviceSurfacePresentModesKHR( gpu.mGPUHandle, mWindowSurface, &numPresentModes, gpu.mPresentModes.data() ) );
			}

			// physical device info
			{
				VkPhysicalDeviceProperties gpuProperties = {};
				vkGetPhysicalDeviceFeatures( gpu.mGPUHandle, &gpu.mFeatures );
				vkGetPhysicalDeviceProperties( gpu.mGPUHandle, &gpu.mProperties );
				vkGetPhysicalDeviceMemoryProperties( gpu.mGPUHandle, &gpu.mMemoryProperties );
			}

			// queue families
			{
				u32 numQueueFamilies = 0;
				vkGetPhysicalDeviceQueueFamilyProperties( gpu.mGPUHandle, &numQueueFamilies, nullptr );
				assertf( numQueueFamilies > 0, "VulkanContext::SelectPhysicalDevice() failed to the queue families of your GPU!\n" );
				gpu.mQueueFamilyProperties.resize( numQueueFamilies );
				vkGetPhysicalDeviceQueueFamilyProperties( gpu.mGPUHandle, &numQueueFamilies, gpu.mQueueFamilyProperties.data() );
			}
		}

#if MSTD_DEBUG
		// get and display all the instance layers
		u32 numInstanceLayers = 0;
		YETI_VK_CHECK( vkEnumerateInstanceLayerProperties( &numInstanceLayers, nullptr ) );
		array<VkLayerProperties> instanceLayers( numInstanceLayers );
		YETI_VK_CHECK( vkEnumerateInstanceLayerProperties( &numInstanceLayers, instanceLayers.data() ) );

		for ( VkLayerProperties layerProperty : instanceLayers ) {
			printf( "%s:\t\t%s\n", layerProperty.layerName, layerProperty.description );
		}

		printf( "\n" );
#endif
	}

	// select physical device
	{
		for ( u32 i = 0; i < gpus.length(); i++ ) {
			gpuInfo_t& gpu = gpus[i];

			// graphics queue
			for ( u32 j = 0; j < gpu.mQueueFamilyProperties.length(); j++ ) {
				VkQueueFamilyProperties& familyProperties = gpu.mQueueFamilyProperties[j];

				if ( familyProperties.queueCount == 0 ) {
					continue;
				}

				if ( familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
					mQueueFamilyIndices[YETI_QUEUE_TYPE_GRAPHICS] = j;
					mActiveGPU = gpus[i];
					break;
				}
			}

			// present queue
			for ( u32 j = 0; j < gpu.mQueueFamilyProperties.length(); j++ ) {
				if ( gpu.mQueueFamilyProperties[j].queueCount == 0 ) {
					continue;
				}

				VkBool32 supportsPresentQueue = VK_FALSE;
				YETI_VK_CHECK( vkGetPhysicalDeviceSurfaceSupportKHR( gpu.mGPUHandle, j, mWindowSurface, &supportsPresentQueue ) );
				if ( supportsPresentQueue ) {
					mQueueFamilyIndices[YETI_QUEUE_TYPE_PRESENT] = j;
					break;
				}
			}
		}

#if MSTD_DEBUG
		// query the device layers
		u32 numDeviceLayerProperties = 0;
		YETI_VK_CHECK( vkEnumerateDeviceLayerProperties( mActiveGPU.mGPUHandle, &numDeviceLayerProperties, nullptr ) );
		assertf( numDeviceLayerProperties > 0, "VulkanContext::SelectPhysicalDevice() failed to find the device layer properties of your GPU!\n" );
		array<VkLayerProperties> deviceLayerProperties( numDeviceLayerProperties );
		YETI_VK_CHECK( vkEnumerateDeviceLayerProperties( mActiveGPU.mGPUHandle, &numDeviceLayerProperties, deviceLayerProperties.data() ) );

		printf( "Vulkan Device Layers:\n" );
		for ( VkLayerProperties layerProperty : deviceLayerProperties ) {
			printf( "%s:\t\t%s\n", layerProperty.layerName, layerProperty.description );
		}

		printf( "\n" );
#endif
	}
}

/*
========================
VulkanContext::CreateLogicalDevice
========================
*/
void VulkanContext::CreateLogicalDevice() {
	array<const char*> deviceExtensions {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	float32 queuePriorities[1] = { 1.0f };

	u32 queueIndexGraphics = mQueueFamilyIndices[YETI_QUEUE_TYPE_GRAPHICS];
	u32 queueIndexPresent = mQueueFamilyIndices[YETI_QUEUE_TYPE_PRESENT];
	array<u32> queueFamilyIndices = { queueIndexGraphics, queueIndexPresent };

	u32 numUniqueQueues = ( queueIndexGraphics != queueIndexPresent ) ? 2 : 1;
	array<VkDeviceQueueCreateInfo> deviceQueueInfos( numUniqueQueues );

	for ( size_t i = 0; i < numUniqueQueues; i++ ) {
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pQueuePriorities = queuePriorities;
		queueInfo.queueCount = 1;
		queueInfo.queueFamilyIndex = mQueueFamilyIndices[i];

		deviceQueueInfos[i] = queueInfo;
	}

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = static_cast<u32>( deviceQueueInfos.length() );
	deviceInfo.pQueueCreateInfos = deviceQueueInfos.data();
	deviceInfo.pEnabledFeatures = &mActiveGPU.mFeatures;
	deviceInfo.enabledExtensionCount = static_cast<u32>( deviceExtensions.length() );
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	YETI_VK_CHECK( vkCreateDevice( mActiveGPU.mGPUHandle, &deviceInfo, nullptr, &mLogicalDevice ) );

	for ( size_t i = 0; i < YETI_QUEUE_TYPE_COUNT; i++ ) {
		vkGetDeviceQueue( mLogicalDevice, mQueueFamilyIndices[i], 0, &mQueues[i] );
	}
}

/*
========================
VulkanContext::DestroyLogicalDevice
========================
*/
void VulkanContext::DestroyLogicalDevice() {
	vkDestroyDevice( mLogicalDevice, nullptr );
	mLogicalDevice = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::CreateAllocator
========================
*/
void VulkanContext::CreateAllocator() {
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.device = mLogicalDevice;
	allocatorInfo.physicalDevice = mActiveGPU.mGPUHandle;
	YETI_VK_CHECK( vmaCreateAllocator( &allocatorInfo, &mAllocator ) );
}

/*
========================
VulkanContext::DestroyAllocator
========================
*/
void VulkanContext::DestroyAllocator() {
	vmaDestroyAllocator( mAllocator );
	mAllocator = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::CreateSwapChain
========================
*/
void VulkanContext::CreateSwapChain() {
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;	// FIFO is default

	// swap chain
	{
		VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		u32* queueFamilyIndices = nullptr;
		u32 queueFamilyIndexCount = 0;

		// surface format
		{
			VkSurfaceFormatKHR preferredFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
			bool32 foundPreferred = false;

			for ( size_t i = 0; i < mActiveGPU.mWindowSurfaceFormats.length(); i++ ) {
				VkSurfaceFormatKHR surfaceFormat = mActiveGPU.mWindowSurfaceFormats[i];

				if ( surfaceFormat.format == preferredFormat.format && surfaceFormat.colorSpace == preferredFormat.colorSpace ) {
					mSurfaceFormat = preferredFormat;
					foundPreferred = true;
					break;
				}
			}

			if ( !foundPreferred ) {
				mSurfaceFormat = mActiveGPU.mWindowSurfaceFormats[0];
			}
		}

		// present mode
		{
			VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;

			for ( size_t i = 0; i < mActiveGPU.mPresentModes.length(); i++ ) {
				if ( mActiveGPU.mPresentModes[i] == preferredPresentMode ) {
					presentMode = mActiveGPU.mPresentModes[i];
					break;
				}
			}
		}

		// sharing mode
		{
			u32 queueIndexGraphics = mQueueFamilyIndices[YETI_QUEUE_TYPE_GRAPHICS];
			u32 queueIndexPresent = mQueueFamilyIndices[YETI_QUEUE_TYPE_PRESENT];

			if ( queueIndexGraphics != queueIndexPresent ) {
				sharingMode = VK_SHARING_MODE_CONCURRENT;

				u32 queueFamilyIndicesList[] = { queueIndexGraphics, queueIndexPresent };
				queueFamilyIndices = queueFamilyIndicesList;

				queueFamilyIndexCount = 2;
			}
		}

		VkSwapchainCreateInfoKHR swapChainInfo = {};
		swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapChainInfo.clipped = VK_TRUE;
		swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapChainInfo.imageArrayLayers = 1;
		swapChainInfo.imageColorSpace = mSurfaceFormat.colorSpace;
		swapChainInfo.imageFormat = mSurfaceFormat.format;
		swapChainInfo.presentMode = presentMode;
		swapChainInfo.imageExtent.width = mWidth;
		swapChainInfo.imageExtent.height = mHeight;
		swapChainInfo.imageSharingMode = sharingMode;
		swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapChainInfo.minImageCount = mNumBuffers;
		swapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
		swapChainInfo.queueFamilyIndexCount = queueFamilyIndexCount;
		swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapChainInfo.surface = mWindowSurface;
		YETI_VK_CHECK( vkCreateSwapchainKHR( mLogicalDevice, &swapChainInfo, nullptr, &mSwapChain ) );
	}

	// swap chain images
	{
		u32 numImages = 0;
		YETI_VK_CHECK( vkGetSwapchainImagesKHR( mLogicalDevice, mSwapChain, &numImages, nullptr ) );
		assert( numImages > 0 );
		mSwapChainImages.resize( numImages );
		mSwapChainImageViews.resize( numImages );
		mFramebuffers.resize( numImages );
		mCommandBuffers.resize( numImages );
		mFences.resize( numImages );
		mCommandBuffersSubmitted.resize( numImages );
		memset( mCommandBuffersSubmitted.data(), 0, sizeof( bool32 ) * mCommandBuffersSubmitted.length() );
		YETI_VK_CHECK( vkGetSwapchainImagesKHR( mLogicalDevice, mSwapChain, &numImages, mSwapChainImages.data() ) );

		for ( size_t i = 0; i < numImages; i++ ) {
			VkImageViewCreateInfo imageViewInfo = {};
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.components = {};
			imageViewInfo.format = mSurfaceFormat.format;
			imageViewInfo.image = mSwapChainImages[i];
			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			YETI_VK_CHECK( vkCreateImageView( mLogicalDevice, &imageViewInfo, nullptr, &mSwapChainImageViews[i] ) );
		}
	}
}

/*
========================
VulkanContext::DestroySwapChain
========================
*/
void VulkanContext::DestroySwapChain() {
	for ( size_t i = 0; i < mSwapChainImageViews.length(); i++ ) {
		VkImageView& imageView = mSwapChainImageViews[i];

		vkDestroyImageView( mLogicalDevice, imageView, nullptr );
		imageView = VK_NULL_HANDLE;
	}

	vkDestroySwapchainKHR( mLogicalDevice, mSwapChain, nullptr );
	mSwapChain = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::RecreateSwapChain
========================
*/
void VulkanContext::RecreateSwapChain() {
	WaitDeviceIdle();

	DestroyCommandBuffers();

	DestroyFramebuffers();

	DestroySwapChain();

	CreateSwapChain();

	CreateFramebuffers();

	CreateCommandBuffers();
}

/*
========================
VulkanContext::CreateRenderPass
========================
*/
void VulkanContext::CreateRenderPass() {
	VkAttachmentDescription attachmentDesc = {};
	attachmentDesc.format = mSurfaceFormat.format;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkAttachmentReference attachmentColor = {};
	attachmentColor.attachment = 0;
	attachmentColor.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &attachmentColor;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDesc;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDesc;
	YETI_VK_CHECK( vkCreateRenderPass( mLogicalDevice, &renderPassInfo, nullptr, &mRenderPass ) );
}

/*
========================
VulkanContext::DestroyRenderPass
========================
*/
void VulkanContext::DestroyRenderPass() {
	vkDestroyRenderPass( mLogicalDevice, mRenderPass, VK_NULL_HANDLE );
	mRenderPass = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::CreateFramebuffers
========================
*/
void VulkanContext::CreateFramebuffers() {
	for ( size_t i = 0; i < mFramebuffers.length(); i++ ) {
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &mSwapChainImageViews[i];
		framebufferInfo.width = mWidth;
		framebufferInfo.height = mHeight;
		framebufferInfo.layers = 1;
		framebufferInfo.renderPass = mRenderPass;
		YETI_VK_CHECK( vkCreateFramebuffer( mLogicalDevice, &framebufferInfo, nullptr, &mFramebuffers[i] ) );
	}
}

/*
========================
VulkanContext::DestroyFramebuffers
========================
*/
void VulkanContext::DestroyFramebuffers() {
	for ( size_t i = 0; i < mFramebuffers.length(); i++ ) {
		VkFramebuffer& framebuffer = mFramebuffers[i];

		vkDestroyFramebuffer( mLogicalDevice, framebuffer, nullptr );
		framebuffer = VK_NULL_HANDLE;
	}
}

/*
========================
VulkanContext::CreateCommandBuffers
========================
*/
void VulkanContext::CreateCommandBuffers() {
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = mQueueFamilyIndices[YETI_QUEUE_TYPE_GRAPHICS];
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	YETI_VK_CHECK( vkCreateCommandPool( mLogicalDevice, &commandPoolInfo, nullptr, &mCommandPool ) );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandBufferCount = static_cast<u32>( mCommandBuffers.length() );
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	YETI_VK_CHECK( vkAllocateCommandBuffers( mLogicalDevice, &allocInfo, mCommandBuffers.data() ) );
}

/*
========================
VulkanContext::DestroyCommandBuffers
========================
*/
void VulkanContext::DestroyCommandBuffers() {
	vkFreeCommandBuffers( mLogicalDevice, mCommandPool, static_cast<u32>( mCommandBuffers.length() ), mCommandBuffers.data() );

	vkDestroyCommandPool( mLogicalDevice, mCommandPool, nullptr );
	mCommandPool = VK_NULL_HANDLE;
}

/*
========================
VulkanContext::CreateSemaphores
========================
*/
void VulkanContext::CreateSemaphores() {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	YETI_VK_CHECK( vkCreateSemaphore( mLogicalDevice, &semaphoreInfo, nullptr, &mSemaphoreAcquireImage ) );
	YETI_VK_CHECK( vkCreateSemaphore( mLogicalDevice, &semaphoreInfo, nullptr, &mSemaphoreRenderComplete ) );
}

/*
========================
VulkanContext::DestroySemaphores
========================
*/
void VulkanContext::DestroySemaphores() {
	vkDestroySemaphore( mLogicalDevice, mSemaphoreRenderComplete, nullptr );
	vkDestroySemaphore( mLogicalDevice, mSemaphoreAcquireImage, nullptr );
}

/*
========================
VulkanContext::CreateFences
========================
*/
void VulkanContext::CreateFences() {
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	for ( size_t i = 0; i < mFences.length(); i++ ) {
		YETI_VK_CHECK( vkCreateFence( mLogicalDevice, &fenceInfo, nullptr, &mFences[i] ) );
	}
}

/*
========================
VulkanContext::DestroyFences
========================
*/
void VulkanContext::DestroyFences() {
	for ( size_t i = 0; i < mFences.length(); i++ ) {
		VkFence& fence = mFences[i];

		vkDestroyFence( mLogicalDevice, fence, nullptr );
		fence = VK_NULL_HANDLE;
	}
}

/*
========================
VulkanContext::CreateQueryPool
========================
*/
void VulkanContext::CreateQueryPool() {
	VkQueryPoolCreateInfo queryPoolInfo = {};
	queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
	queryPoolInfo.queryCount = 2;
	YETI_VK_CHECK( vkCreateQueryPool( mLogicalDevice, &queryPoolInfo, nullptr, &mQueryPoolTimestamps ) );
}

/*
========================
VulkanContext::DestroyQueryPool
========================
*/
void VulkanContext::DestroyQueryPool() {
	vkDestroyQueryPool( mLogicalDevice, mQueryPoolTimestamps, nullptr );
	mQueryPoolTimestamps = VK_NULL_HANDLE;
}