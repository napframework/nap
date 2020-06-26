#include "renderwindow.h"
#include "sdlhelpers.h"

#include <windowevent.h>
#include <renderservice.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <SDL_vulkan.h>

RTTI_BEGIN_ENUM(nap::RenderWindow::EPresentationMode)
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::Immediate,	"Immediate"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::Mailbox,		"Mailbox"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::FIFO,			"FIFO")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderWindow)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Borderless",		&nap::RenderWindow::mBorderless,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Resizable",		&nap::RenderWindow::mResizable,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Visible",		&nap::RenderWindow::mVisible,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleShading",	&nap::RenderWindow::mSampleShading,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Title",			&nap::RenderWindow::mTitle,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Width",			&nap::RenderWindow::mWidth,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Height",			&nap::RenderWindow::mHeight,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Mode",			&nap::RenderWindow::mMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor",		&nap::RenderWindow::mClearColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Samples",		&nap::RenderWindow::mRequestedSamples,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static SDL Functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Creates a new SDL window based on the settings provided by the render window
	 * @return: the create window, nullptr if not successful
	 */
	static SDL_Window* createSDLWindow(const RenderWindow& renderWindow, nap::utility::ErrorState& errorState)
	{
		// Construct options
		Uint32 options = SDL_WINDOW_VULKAN;
		options = renderWindow.mResizable  ? options | SDL_WINDOW_RESIZABLE : options;
		options = renderWindow.mBorderless ? options | SDL_WINDOW_BORDERLESS : options;
		options = !renderWindow.mVisible ? options | SDL_WINDOW_HIDDEN : options;
		options = options | SDL_WINDOW_ALLOW_HIGHDPI;

		SDL_Window* new_window = SDL_CreateWindow(renderWindow.mTitle.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			renderWindow.mWidth,
			renderWindow.mHeight,
			options);

		if (!errorState.check(new_window != nullptr, "Failed to create window: %s", SDL::getSDLError().c_str()))
			return nullptr;

		return new_window;
	}


	//////////////////////////////////////////////////////////////////////////
	// Static Vulkan Functions
	//////////////////////////////////////////////////////////////////////////

	/**
	*	Creates the vulkan surface that is rendered to by the device using SDL
	*/
	static bool createSurface(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, uint32_t graphicsFamilyQueueIndex, VkSurfaceKHR& outSurface, utility::ErrorState& errorState)
	{
		if (!errorState.check(SDL_Vulkan_CreateSurface(window, instance, &outSurface) == SDL_TRUE, "Unable to create Vulkan compatible surface using SDL"))
			return false;

		// Make sure the surface is compatible with the queue family and gpu
		VkBool32 supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, graphicsFamilyQueueIndex, outSurface, &supported);
		if (!errorState.check(supported != 0, "Surface is not supported by physical device"))
			return false;

		return true;
	}


	/**
	 * Obtain the surface properties that are required for the creation of the swap chain
	 */
	static bool getSurfaceProperties(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& capabilities, utility::ErrorState& errorState)
	{
		if (!errorState.check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities) == VK_SUCCESS, "Unable to acquire surface capabilities"))
			return false;

		return true;
	}


	/**
	 * Returns if the requested presentation mode is supported, fall-back = FIFO_KHR
	 */
	static bool getPresentationMode(VkSurfaceKHR surface, VkPhysicalDevice device, VkPresentModeKHR requestedMode, VkPresentModeKHR& outMode, utility::ErrorState& errorState)
	{
		uint32_t mode_count(0);
		if (!errorState.check(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, NULL) == VK_SUCCESS, "Unable to query present mode count for physical device"))
			return false;

		std::vector<VkPresentModeKHR> available_modes(mode_count);
		if (!errorState.check(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, available_modes.data()) == VK_SUCCESS, "Unable to query the various present modes for physical device"))
			return false;

		for (auto& mode : available_modes)
		{
			if (mode == requestedMode)
			{
				outMode = requestedMode;
				return true;
			}
		}

		// Fall back on FIFO if requested mode is not supported
		outMode = VK_PRESENT_MODE_FIFO_KHR;
		return true;
	}


	/**
	* Figure out the number of images that are used by the swapchain and
	* available to us in the application, based on the minimum amount of necessary images
	* provided by the capabilities struct.
	*/
	static unsigned int getNumberOfSwapImages(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		unsigned int number = capabilities.minImageCount + 1;
		return number > capabilities.maxImageCount ? capabilities.minImageCount : number;
	}


	/**
	*	Returns the size of a swapchain image based on the current surface
	*/
	static VkExtent2D getSwapImageSize(glm::ivec2 windowSize, const VkSurfaceCapabilitiesKHR& capabilities)
	{
		// Default size = window size
		VkExtent2D size = { (unsigned int)windowSize.x, (unsigned int)windowSize.y };

		// This happens when the window scales based on the size of an image
		if (capabilities.currentExtent.width == 0xFFFFFFF)
		{
			size.width = glm::clamp<unsigned int>(size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			size.height = glm::clamp<unsigned int>(size.height, capabilities.maxImageExtent.height, capabilities.maxImageExtent.height);
		}
		else
		{
			size = capabilities.currentExtent;
		}
		return size;
	}


	/**
	* Checks if the surface supports color and other required surface bits
	* If so constructs a ImageUsageFlags bitmask that is returned in outUsage
	* @return if the surface supports all the previously defined bits
	*/
	static bool getImageUsage(const VkSurfaceCapabilitiesKHR& capabilities, VkImageUsageFlags& outUsage, utility::ErrorState& errorState)
	{
		const std::vector<VkImageUsageFlags>& desired_usages{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		assert(desired_usages.size() > 0);

		// Needs to be always present
		outUsage = desired_usages[0];

		for (const auto& desired_usage : desired_usages)
		{
			VkImageUsageFlags image_usage = desired_usage & capabilities.supportedUsageFlags;
			if (!errorState.check(image_usage == desired_usage, "Unsupported image usage flag: %d", desired_usage))
				return false;

			// Add bit if found as supported color
			outUsage = (outUsage | desired_usage);
		}

		return true;
	}


	/**
	* @return transform based on global declared above, current transform if that transform isn't available
	*/
	static VkSurfaceTransformFlagBitsKHR getTransform(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		return capabilities.currentTransform;
	}


	/**
	* @return the most appropriate color space
	*/
	static bool getFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR& outFormat, utility::ErrorState& errorState)
	{
		unsigned int count(0);
		if (!errorState.check(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr) == VK_SUCCESS, "Unable to query number of supported surface formats"))
			return false;

		std::vector<VkSurfaceFormatKHR> found_formats(count);
		if (!errorState.check(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, found_formats.data()) == VK_SUCCESS, "Unable to query all supported surface formats"))
			return false;

		// This means there are no restrictions on the supported format.
		// Preference would work
		if (found_formats.size() == 1 && found_formats[0].format == VK_FORMAT_UNDEFINED)
		{
			outFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
			outFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			return true;
		}

		// Otherwise check if both are supported
		for (const auto& found_format_outer : found_formats)
		{
			// Format found
			if (found_format_outer.format == VK_FORMAT_B8G8R8A8_UNORM)		// TODO: we expect this should be VK_FORMAT_B8G8R8A8_SRGB, but it appears that the previous OpenGl implementation didn't use it, so some colors could be washed out
			{
				outFormat.format = found_format_outer.format;
				for (const auto& found_format_inner : found_formats)
				{
					// Color space found
					if (found_format_inner.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					{
						outFormat.colorSpace = found_format_inner.colorSpace;
						return true;
					}
				}

				// No matching color space, pick first one
				Logger::info("Warning: no matching color space found, picking first available one");
				outFormat.colorSpace = found_formats[0].colorSpace;
				return true;
			}
		}

		// No matching formats found
		Logger::info("Warning: no matching color space found, picking first available one");
		outFormat = found_formats[0];
		return true;
	}


	/**
	* creates the swap chain using utility functions above to retrieve swap chain properties
	* Swap chain is associated with a single window (surface) and allows us to display images to screen
	*/
	static bool createSwapChain(glm::ivec2 windowSize, VkPresentModeKHR presentMode, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, VkSwapchainKHR& outSwapChain, VkExtent2D& outSwapChainExtent, VkFormat& outSwapChainFormat, VkPresentModeKHR& outPresentMode, utility::ErrorState& errorState)
	{
		// Get properties of surface, necessary for creation of swap-chain
		VkSurfaceCapabilitiesKHR surface_properties;
		if (!getSurfaceProperties(physicalDevice, surface, surface_properties, errorState))
			return false;

		// Get the image presentation mode (synced, immediate etc.)
		if (!getPresentationMode(surface, physicalDevice, presentMode, outPresentMode, errorState))
			return false;

		// Get other swap chain related features
		unsigned int swap_image_count = getNumberOfSwapImages(surface_properties);

		// Size of the images
		outSwapChainExtent = getSwapImageSize(windowSize, surface_properties);

		// Get image usage (color etc.)
		VkImageUsageFlags usage_flags;
		if (!getImageUsage(surface_properties, usage_flags, errorState))
			return false;

		// Get the transform, falls back on current transform when transform is not supported
		VkSurfaceTransformFlagBitsKHR transform = getTransform(surface_properties);

		// Get swapchain image format
		VkSurfaceFormatKHR image_format;
		if (!getFormat(physicalDevice, surface, image_format, errorState))
			return false;

		// Populate swapchain creation info
		VkSwapchainCreateInfoKHR swap_info;
		swap_info.pNext = nullptr;
		swap_info.flags = 0;
		swap_info.surface = surface;
		swap_info.minImageCount = swap_image_count;
		swap_info.imageFormat = image_format.format;
		swap_info.imageColorSpace = image_format.colorSpace;
		swap_info.imageExtent = outSwapChainExtent;
		swap_info.imageArrayLayers = 1;
		swap_info.imageUsage = usage_flags;
		swap_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_info.queueFamilyIndexCount = 0;
		swap_info.pQueueFamilyIndices = nullptr;
		swap_info.preTransform = transform;
		swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swap_info.presentMode = outPresentMode;
		swap_info.clipped = true;
		swap_info.oldSwapchain = NULL;
		swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		// Create new one
		if (!errorState.check(vkCreateSwapchainKHR(device, &swap_info, nullptr, &outSwapChain) == VK_SUCCESS, "Unable to create swap chain"))
			return false;

		// Store handle
		outSwapChainFormat = image_format.format;
		return true;
	}


	/**
	 * Returns the handles of all the images in a swap chain, result is stored in outImageHandles
	 */
	static bool getSwapChainImageHandles(VkDevice device, VkSwapchainKHR chain, std::vector<VkImage>& outImageHandles, utility::ErrorState& errorState)
	{
		unsigned int image_count(0);
		if (!errorState.check(vkGetSwapchainImagesKHR(device, chain, &image_count, nullptr) == VK_SUCCESS, "Unable to get number of images in swap chain"))
			return false;

		outImageHandles.clear();
		outImageHandles.resize(image_count);
		if (!errorState.check(vkGetSwapchainImagesKHR(device, chain, &image_count, outImageHandles.data()) == VK_SUCCESS, "Unable to get image handles from swap chain"))
			return false;

		return true;
	}


	static bool createRenderPass(VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat, VkSampleCountFlagBits samples, VkRenderPass& renderPass, utility::ErrorState& errorState)
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = samples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = samples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 3> attachments = { colorAttachment, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		return errorState.check(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
	}


	static bool createSwapchainImageViews(VkDevice device, std::vector<VkImageView>& swapChainImageViews, const std::vector<VkImage>& swapChainImages, VkFormat swapChainFormat, utility::ErrorState& errorState)
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			if (!create2DImageView(device, swapChainImages[i], swapChainFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i], errorState))
				return false;
		}
		return true;
	}


	static bool createFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers, VkImageView colorImageView, VkImageView depthImageView, std::vector<VkImageView>& swapChainImageViews, VkRenderPass renderPass, VkExtent2D extent, utility::ErrorState& errorState)
	{
		// Create a frame buffer for every view in the swapchain.
		framebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments =
			{
				colorImageView,
				depthImageView,
				swapChainImageViews[i],
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (!errorState.check(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
				return false;
		}
		return true;
	}


	static bool createCommandBuffers(VkDevice device, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers, int inNumCommandBuffers, utility::ErrorState& errorState)
	{
		commandBuffers.resize(inNumCommandBuffers);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		return errorState.check(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) == VK_SUCCESS, "Failed to alocate command buffers");
	}

	bool createSyncObjects(VkDevice device, std::vector<VkSemaphore>& imageAvailableSemaphores, std::vector<VkSemaphore>& renderFinishedSemaphores, int numFramesInFlight, utility::ErrorState& errorState)
	{
		imageAvailableSemaphores.resize(numFramesInFlight);
		renderFinishedSemaphores.resize(numFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (size_t i = 0; i < numFramesInFlight; i++)
		{
			if (!errorState.check(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) == VK_SUCCESS &&
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) == VK_SUCCESS, "Failed to create sync objects"))
			{
				return false;
			}
		}

		return true;
	}


	static unsigned int findPresentFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (int index = 0; index < queueFamilies.size(); ++index)
		{
			VkQueueFamilyProperties& queueFamily = queueFamilies[index];

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surface, &presentSupport);

			if (queueFamily.queueCount > 0 && presentSupport)
				return index;
		}

		return -1;
	}

	static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		return -1;
	}


	static bool createColorResource(const RenderService& renderer, VkExtent2D swapchainExtent, VkFormat colorFormat, VkSampleCountFlagBits sampleCount, ImageData& outData, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), swapchainExtent.width, swapchainExtent.height, colorFormat, 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outData.mTextureImage, outData.mTextureAllocation, outData.mTextureAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outData.mTextureImage, colorFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT, outData.mTextureView, errorState))
			return false;

		return true;
	}


	static bool createDepthResource(const RenderService& renderer, VkExtent2D swapchainExtent, VkSampleCountFlagBits sampleCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), swapchainExtent.width, swapchainExtent.height, renderer.getDepthFormat(), 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outImage.mTextureImage, outImage.mTextureAllocation, outImage.mTextureAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.mTextureImage, renderer.getDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT, outImage.mTextureView, errorState))
			return false;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Member Functions
	//////////////////////////////////////////////////////////////////////////


	RenderWindow::RenderWindow(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	RenderWindow::~RenderWindow()
	{
		// Return immediately if there's no actual native present present
		// This is the case when the window is created but not initialized or did not initialize properly
		if (mSDLWindow == nullptr)
			return;

		// Wait for device to go idle before destroying the window-related resources
		if (mDevice != nullptr)
		{
			VkResult result = vkDeviceWaitIdle(mDevice);
			assert(result == VK_SUCCESS);
		}

		// Destroy all vulkan resources if present
		for (VkSemaphore semaphore : mImageAvailableSemaphores)
			vkDestroySemaphore(mDevice, semaphore, nullptr);

		for (VkSemaphore semaphore : mRenderFinishedSemaphores)
			vkDestroySemaphore(mDevice, semaphore, nullptr);

		if (!mCommandBuffers.empty())
			vkFreeCommandBuffers(mDevice, mRenderService->getCommandPool(), mCommandBuffers.size(), mCommandBuffers.data());

		// Destroy all resources associated with swapchain
		destroySwapChainResources();
		if (mSurface != nullptr)
			vkDestroySurfaceKHR(mRenderService->getVulkanInstance(), mSurface, nullptr);

		// Destroy SDL Window
		SDL_DestroyWindow(mSDLWindow);
	}


	bool RenderWindow::init(utility::ErrorState& errorState)
	{
		// Create SDL window first
		assert(mSDLWindow == nullptr);
		mSDLWindow = createSDLWindow(*this, errorState);
		if (mSDLWindow == nullptr)
			return false;

		// Fetch required vulkan handles
		mDevice = mRenderService->getDevice();

		// Set size and store for future reference
		setSize(glm::vec2(mWidth, mHeight));
		mPreviousWindowSize = glm::ivec2(mWidth, mHeight);

		// Acquire max number of MSAA samples, issue warning if requested number
		// of samples is not obtained.
		utility::ErrorState rast_error;
		if (!mRenderService->getRasterizationSamples(mRequestedSamples, mRasterizationSamples, rast_error))
			nap::Logger::warn(rast_error.toString().c_str());

		// Check if sample rate shading is enabled and supported
		mSampleShadingEnabled = mSampleShading;
		if (mSampleShadingEnabled && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShadingEnabled = false;
		}

		// Convert presentation mode
		mPresentationMode = mMode == RenderWindow::EPresentationMode::FIFO ? 
			VK_PRESENT_MODE_FIFO_RELAXED_KHR :
			mMode == RenderWindow::EPresentationMode::Mailbox ? 
			VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

		// acquire handle to physical device
		VkPhysicalDevice physicalDevice = mRenderService->getPhysicalDevice();

		// Create render surface for window
		if (!createSurface(mSDLWindow, mRenderService->getVulkanInstance(), physicalDevice, mRenderService->getGraphicsQueueIndex(), mSurface, errorState))
			return false;

		// Create swapchain and associated resources
		if (!createSwapChainResources(errorState))
			return false;

		// Create required command buffers for max frames in flight
		if (!createCommandBuffers(mDevice, mRenderService->getCommandPool(), mCommandBuffers, mRenderService->getMaxFramesInFlight(), errorState))
			return false;

		// Create frame / GPU synchronization objects
		if (!createSyncObjects(mDevice, mImageAvailableSemaphores, mRenderFinishedSemaphores, mRenderService->getMaxFramesInFlight(), errorState))
			return false;

		// Get presentation queue
		unsigned int presentQueueIndex = findPresentFamilyIndex(physicalDevice, mSurface);
		if (!errorState.check(presentQueueIndex != -1, "Failed to find present queue"))
			return false;

		// Get queue
		vkGetDeviceQueue(mDevice, presentQueueIndex, 0, &mPresentQueue);

		// Add window to render service
		if (!mRenderService->addWindow(*this, errorState))
			return false;

		// We want to respond to resize events for this window
		mWindowEvent.connect(std::bind(&RenderWindow::handleEvent, this, std::placeholders::_1));

		return true;
	}
    
    
	void RenderWindow::onDestroy()
	{
		mRenderService->removeWindow(*this);
	}
	

	void RenderWindow::show()
	{
		SDL::showWindow(mSDLWindow, true);
		SDL::raiseWindow(mSDLWindow);
	}


	void RenderWindow::hide()
	{
		SDL::showWindow(mSDLWindow, false);
	}


	void RenderWindow::setFullscreen(bool value)
	{
		SDL::setFullscreen(mSDLWindow, value);
		mFullscreen = value;
	}


	void RenderWindow::toggleFullscreen()
	{
		setFullscreen(!mFullscreen);
	}


	void RenderWindow::setWidth(int width)
	{
		setSize({ width, getSize().y });
	}


	void RenderWindow::setHeight(int height)
	{
		setSize({ getSize().x, height });
	}


	void RenderWindow::setSize(const glm::ivec2& size)
	{
		SDL::setWindowSize(mSDLWindow, size);
	}


	const glm::ivec2 RenderWindow::getBufferSize() const
	{
		return SDL::getDrawableWindowSize(mSDLWindow);
	}


	const glm::ivec2 RenderWindow::getSize() const
	{
		return SDL::getWindowSize(mSDLWindow);
	}


	void RenderWindow::setClearColor(const glm::vec4& color)
	{
		mClearColor = color;
	}


	const glm::vec4& RenderWindow::getClearColor() const
	{
		return mClearColor;
	}


	void RenderWindow::setPosition(const glm::ivec2& position)
	{
		SDL::setWindowPosition(mSDLWindow, position);
	}


	const glm::ivec2 RenderWindow::getPosition() const
	{
		return SDL::getWindowPosition(mSDLWindow);
	}


	SDL_Window* RenderWindow::getNativeWindow() const
	{
		return mSDLWindow;
	}


	uint RenderWindow::getNumber() const
	{
		return static_cast<uint>(SDL::getWindowId(getNativeWindow()));
	}


	math::Rect RenderWindow::getRect() const
	{
		return { 0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()) };
	}


	math::Rect RenderWindow::getRectPixels() const
	{
		return{ 0.0f, 0.0f, static_cast<float>(getWidthPixels()), static_cast<float>(getHeightPixels()) };
	}


	VkCommandBuffer RenderWindow::makeActive()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();

		glm::ivec2 window_size = SDL::getWindowSize(mSDLWindow);
		uint32_t window_state = SDL_GetWindowFlags(mSDLWindow);

		// Check if the window has a zero size. Note that in the case where the window is minimized, it seems SDL still reports
		// the window as having a non-zero size. However, Vulkan internally knows this is not the case (it sees it as a zero-sized window), which will result in 
		// errors being thrown by vkAcquireNextImageKHR etc if we try to render anyway. So, to workaround this issue, we also consider minimized windows to be of zero size.
		//
		// In either case, when the window is zero-sized, we can't render to it since there is no valid swap chain. So, we return a nullptr to signal this to the client.
		bool is_zero_size_window = window_size.x == 0 || window_size.y == 0 || (window_state & SDL_WINDOW_MINIMIZED) != 0;
		if (is_zero_size_window)
			return nullptr;

		// When the window size has changed, we need to recreate the swapchain.
		//
		// Note that vkAcquireNextImageKHR and vkQueuePresentKHR can return VK_ERROR_OUT_OF_DATE_KHR, which is used to signal that the framebuffer (size or format) no longer matches 
		// the swapchain. This can be used to recreate the swapchain. However, in practice we've found that recreating the swap chain at that point results in obscure errors about 
		// resources still being used / device lost error messages. Since our main loop is single threaded, and Window events are processed before update/render, we just deal with resizes here,
		// before we start rendering to the window. It is not possible for the window size to change *during* rendering (because, single threaded), so this keeps it simple.
		if (window_size != mPreviousWindowSize)
		{
			utility::ErrorState errorState;
			if (!recreateSwapChain(errorState))
			{
				Logger::info("Failed to recreate swapchain: %s", errorState.toString().c_str());
				return nullptr;
			}
			mPreviousWindowSize = window_size;
		}

		// According to the spec, vkAcquireNextImageKHR can return VK_ERROR_OUT_OF_DATE_KHR when the framebuffer no longer matches the swapchain.
		// In our case this should only happen due to window size changes, which is handled explicitly above. So, we don't attempt to handle it here.
		VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mImageAvailableSemaphores[current_frame], VK_NULL_HANDLE, &mCurrentImageIndex);
		assert(result == VK_SUCCESS);

		VkCommandBuffer commandBuffer = mCommandBuffers[current_frame];
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		assert(result == VK_SUCCESS);

		return commandBuffer;
	}


	void RenderWindow::swap() const
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();
		VkCommandBuffer commandBuffer = mCommandBuffers[current_frame];

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
			throw std::runtime_error("failed to record command buffer!");

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[current_frame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[current_frame];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[current_frame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VkResult result = vkQueueSubmit(mRenderService->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &mCurrentImageIndex;

		// According to the spec, vkQueuePresentKHR can return VK_ERROR_OUT_OF_DATE_KHR when the framebuffer no longer matches the swapchain.
		// In our case this should only happen due to window size changes, which is handled in makeCurrent. So, we don't attempt to handle it here.
		result = vkQueuePresentKHR(mPresentQueue, &presentInfo);
		assert(result == VK_SUCCESS);
	}


	void RenderWindow::handleEvent(const Event& event)
	{
		// Update window size when resizing
		const WindowResizedEvent* resized_event = rtti_cast<const WindowResizedEvent>(&event);
		if (resized_event != nullptr)
		{
			setSize(glm::ivec2(resized_event->mX, resized_event->mY));
		}
	}


	bool RenderWindow::recreateSwapChain(utility::ErrorState& errorState)
	{
		destroySwapChainResources();
		return createSwapChainResources(errorState);
	}


	bool RenderWindow::createSwapChainResources(utility::ErrorState& errorState)
	{
		// TODO: getSize() needs to be drawable size on OSX, je pense!
		VkExtent2D swapchainExtent;
		VkPresentModeKHR out_mode;
		if (!createSwapChain(getSize(), mPresentationMode, mSurface, mRenderService->getPhysicalDevice(), mDevice, mSwapchain, swapchainExtent, mSwapchainFormat, out_mode, errorState))
			return false;
		
		// Check if the selected presentation mode matches our request
		if (out_mode != mPresentationMode)
		{
			nap::Logger::warn("%s: Unsupported presentation mode, switched to FIFO", mID.c_str());
			mPresentationMode = out_mode;
		}

		// Get image handles from swap chain
		std::vector<VkImage> chain_images;
		if (!getSwapChainImageHandles(mDevice, mSwapchain, chain_images, errorState))
			return false;

		if (!createRenderPass(mDevice, mSwapchainFormat, mRenderService->getDepthFormat(), mRasterizationSamples, mRenderPass, errorState))
			return false;

		if (!createSwapchainImageViews(mDevice, mSwapChainImageViews, chain_images, mSwapchainFormat, errorState))
			return false;

		if (!createDepthResource(*mRenderService, swapchainExtent, mRasterizationSamples, mDepthImage, errorState))
			return false;

		if (!createColorResource(*mRenderService, swapchainExtent, mSwapchainFormat, mRasterizationSamples, mColorImage, errorState))
			return false;

		if (!createFramebuffers(mDevice, mSwapChainFramebuffers, mColorImage.mTextureView, mDepthImage.mTextureView, mSwapChainImageViews, mRenderPass, swapchainExtent, errorState))
			return false;

		return true;
	}


	void RenderWindow::destroySwapChainResources()
	{
		if (mSwapchain == nullptr)
			return;

		VkResult result = vkDeviceWaitIdle(mDevice);
		assert(result == VK_SUCCESS);
		for (VkFramebuffer frame_buffer : mSwapChainFramebuffers)
			vkDestroyFramebuffer(mDevice, frame_buffer, nullptr);

		mSwapChainFramebuffers.clear();
		destroyImageAndView(mDepthImage, mDevice, mRenderService->getVulkanAllocator());
		destroyImageAndView(mColorImage, mDevice, mRenderService->getVulkanAllocator());

		for (VkImageView image_view : mSwapChainImageViews)
			vkDestroyImageView(mDevice, image_view, nullptr);

		mSwapChainImageViews.clear();
		if (mRenderPass != nullptr)
		{
			vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
			mRenderPass = nullptr;
		}

		if (mSwapchain != nullptr)
		{
			vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
			mSwapchain = nullptr;
		}
	}


	void RenderWindow::beginRendering()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();

		// TODO: This could be incorrect on HighDPI monitors, in that case use drawable size
		glm::ivec2 window_size = SDL::getWindowSize(mSDLWindow);
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapChainFramebuffers[mCurrentImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (uint32_t)window_size.x, (uint32_t)window_size.y };

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(mCommandBuffers[current_frame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = window_size.x;
		rect.extent.height = window_size.y;
		vkCmdSetScissor(mCommandBuffers[current_frame], 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = window_size.x;
		viewport.height = window_size.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mCommandBuffers[current_frame], 0, 1, &viewport);
	}


	void RenderWindow::endRendering()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();
		vkCmdEndRenderPass(mCommandBuffers[current_frame]);
	}


	VkFormat RenderWindow::getDepthFormat() const
	{
		return mRenderService->getDepthFormat();
	}


	nap::ECullWindingOrder RenderWindow::getWindingOrder() const
	{
		return ECullWindingOrder::CounterClockwise;
	}
}