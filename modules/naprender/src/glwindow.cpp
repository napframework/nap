// Local includes
#include "glwindow.h"
#include "renderservice.h"
#include "SDL_vulkan.h"
#include "vulkan/vulkan.h"

// External includes
#include <utility/errorstate.h>
#include "sdlhelpers.h"
#include "nap/logger.h"

namespace nap
{
	/**
	* createWindow
	*
	* Creates a new window using the parameters specified
	* @return: the create window, nullptr if not successful
	*/
	static SDL_Window* createSDLWindow(const RenderWindowSettings& settings, nap::utility::ErrorState& errorState)
	{
		// Construct options
		Uint32 options = SDL_WINDOW_VULKAN;
		options = settings.resizable ? options	| SDL_WINDOW_RESIZABLE : options;
		options = settings.borderless ? options | SDL_WINDOW_BORDERLESS : options;
		options = !settings.visible ? options	| SDL_WINDOW_HIDDEN : options;
        options = settings.highdpi ?  options	| SDL_WINDOW_ALLOW_HIGHDPI : options;

		SDL_Window* new_window = SDL_CreateWindow(	settings.title.c_str(),
													settings.x,
													settings.y,
													settings.width,
													settings.height,
													options);

		if (!errorState.check(new_window != nullptr, "Failed to create window: %s", SDL::getSDLError().c_str()))
			return nullptr;

		return new_window;
	}

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
	 * @return if the present modes could be queried and ioMode is set
	 * @param outMode the mode that is requested, will contain FIFO when requested mode is not available
	 */
	static bool getPresentationMode(VkSurfaceKHR surface, VkPhysicalDevice device, VkPresentModeKHR& ioMode, utility::ErrorState& errorState)
	{
		uint32_t mode_count(0);
		if (!errorState.check(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, NULL) == VK_SUCCESS, "Unable to query present mode count for physical device"))
			return false;

		std::vector<VkPresentModeKHR> available_modes(mode_count);
		if (!errorState.check(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, available_modes.data()) == VK_SUCCESS, "Unable to query the various present modes for physical device"))
			return false;

		for (auto& mode : available_modes)
		{
			if (mode == ioMode)
				return true;
		}

		ioMode = VK_PRESENT_MODE_FIFO_KHR;
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
		VkExtent2D size = { (unsigned int)windowSize.x, (unsigned int)windowSize.y};

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
		const std::vector<VkImageUsageFlags>& desired_usages { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
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
	static bool createSwapChain(glm::ivec2 windowSize, VkPresentModeKHR mode, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, VkSwapchainKHR& outSwapChain, VkExtent2D& outSwapChainExtent, VkFormat& outSwapChainFormat, utility::ErrorState& errorState)
	{
		// Get properties of surface, necessary for creation of swap-chain
		VkSurfaceCapabilitiesKHR surface_properties;
		if (!getSurfaceProperties(physicalDevice, surface, surface_properties, errorState))
			return false;

		// Get the image presentation mode (synced, immediate etc.)
		VkPresentModeKHR selected_mode = mode;
		if (!getPresentationMode(surface, physicalDevice, selected_mode, errorState))
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
		swap_info.presentMode = selected_mode;
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
	 *	Returns the handles of all the images in a swap chain, result is stored in outImageHandles
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
			if (!create2DImageView(device, swapChainImages[i], swapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, swapChainImageViews[i], errorState))
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
		// Create image allocation struct
		VmaAllocationCreateInfo img_alloc_usage = {};
		img_alloc_usage.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_alloc_usage.flags = 0;

		if (!create2DImage(renderer.getVulkanAllocator(), swapchainExtent.width, swapchainExtent.height, colorFormat, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, img_alloc_usage, outData.mTextureImage, outData.mTextureAllocation, outData.mTextureAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outData.mTextureImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, outData.mTextureView, errorState))
			return false;

		return true;
	}


	static bool createDepthResource(const RenderService& renderer, VkExtent2D swapchainExtent, VkSampleCountFlagBits sampleCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		// Create image allocation struct
		VmaAllocationCreateInfo img_alloc_usage = {};
		img_alloc_usage.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		img_alloc_usage.flags = 0;

		if (!create2DImage(renderer.getVulkanAllocator(), swapchainExtent.width, swapchainExtent.height, renderer.getDepthFormat(), sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,img_alloc_usage, outImage.mTextureImage, outImage.mTextureAllocation, outImage.mTextureAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.mTextureImage, renderer.getDepthFormat(), VK_IMAGE_ASPECT_DEPTH_BIT, outImage.mTextureView, errorState))
			return false;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	// GL Render window constructor
	GLWindow::GLWindow() :
		mBackbuffer(*this)
	{ 
	}


	GLWindow::~GLWindow()
	{	
		// Wait for device to go idle before destroying the window-related resources
		VkResult result = vkDeviceWaitIdle(mDevice);
		assert(result == VK_SUCCESS);

		for (VkSemaphore semaphore : mImageAvailableSemaphores)
			vkDestroySemaphore(mDevice, semaphore, nullptr);

		for (VkSemaphore semaphore : mRenderFinishedSemaphores)
			vkDestroySemaphore(mDevice, semaphore, nullptr);

		if (!mCommandBuffers.empty())
			vkFreeCommandBuffers(mDevice, mRenderService->getCommandPool(), mCommandBuffers.size(), mCommandBuffers.data());

		destroySwapChainResources();

		if (mSurface != nullptr)
			vkDestroySurfaceKHR(mRenderService->getVulkanInstance(), mSurface, nullptr);

		if (mWindow != nullptr)
			SDL_DestroyWindow(mWindow);
	}


	void GLWindow::applySettings(const RenderWindowSettings& settings)
	{
		setSize(glm::vec2(settings.width, settings.height));
		SDL::setWindowPosition(mWindow, glm::vec2(settings.x, settings.y));
		SDL::setWindowResizable(mWindow, settings.resizable);
		SDL::setWindowBordered(mWindow, !settings.borderless);
		SDL::setWindowTitle(mWindow, settings.title);
		if (settings.visible)
			showWindow();
		else
			hideWindow();
	}

	bool GLWindow::recreateSwapChain(utility::ErrorState& errorState)
	{
		destroySwapChainResources();
		return createSwapChainResources(errorState);
	}

	bool GLWindow::createSwapChainResources(utility::ErrorState& errorState)
	{
		VkExtent2D swapchainExtent;
		if (!createSwapChain(getSize(), mMode, mSurface, mRenderService->getPhysicalDevice(), mDevice, mSwapchain, swapchainExtent, mSwapchainFormat, errorState))
			return false;

		// Get image handles from swap chain
		std::vector<VkImage> chain_images;
		if (!getSwapChainImageHandles(mDevice, mSwapchain, chain_images, errorState))
			return false;

		if (!createRenderPass(mDevice, mSwapchainFormat, mRenderService->getDepthFormat(), getSampleCount(), mRenderPass, errorState))
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

	void GLWindow::destroySwapChainResources()
	{
		if (mSwapchain == nullptr)
			return;
		
		VkResult result = vkDeviceWaitIdle(mDevice);
		assert(result == VK_SUCCESS);

		for (VkFramebuffer frame_buffer : mSwapChainFramebuffers)
			vkDestroyFramebuffer(mDevice, frame_buffer, nullptr);

		mSwapChainFramebuffers.clear();
		destroyImageAndView(mDepthImage, mDevice, mAllocator);
		destroyImageAndView(mColorImage, mDevice, mAllocator);

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

	bool GLWindow::init(const RenderWindowSettings& settings, RenderService& renderService, nap::utility::ErrorState& errorState)
	{
		mRenderService = &renderService;

		// create the window
		mWindow = createSDLWindow(settings, errorState);
		if (mWindow == nullptr)
			return false;

		// Set size and store for future reference
		setSize(glm::vec2(settings.width, settings.height));
		mPreviousWindowSize = glm::ivec2(settings.width, settings.height);

		// Initialize members
		mAllocator = renderService.getVulkanAllocator();
		mDevice = renderService.getDevice();
		mGraphicsQueue = renderService.getGraphicsQueue();
		mMode = settings.mode;
		mRasterizationSamples = settings.samples;

		// Check if sample rate shading is enabled and supported
		mSampleShadingEnabled = settings.sampleShadingEnabled;
		if (mSampleShadingEnabled && !(mRenderService->sampleShadingSupported()))
		{
			nap::Logger::warn("Sample shading requested but not supported");
			mSampleShadingEnabled = false;
		}

		// acquire handle to physical device
		VkPhysicalDevice physicalDevice = renderService.getPhysicalDevice();

		// Create render surface for window
		if (!createSurface(mWindow, renderService.getVulkanInstance(), physicalDevice, renderService.getGraphicsQueueIndex(), mSurface, errorState))
			return false;

		// Create swapchain and associated resources
		if (!createSwapChainResources(errorState))
			return false;

		if (!createCommandBuffers(mDevice, renderService.getCommandPool(), mCommandBuffers, renderService.getMaxFramesInFlight(), errorState))
			return false;

		if (!createSyncObjects(mDevice, mImageAvailableSemaphores, mRenderFinishedSemaphores, renderService.getMaxFramesInFlight(), errorState))
			return false;

		unsigned int presentQueueIndex = findPresentFamilyIndex(physicalDevice, mSurface);
		if (!errorState.check(presentQueueIndex != -1, "Failed to find present queue"))
			return false;

		vkGetDeviceQueue(mDevice, presentQueueIndex, 0, &mPresentQueue);

		return true;
	}


	// Returns the actual SDL window
	SDL_Window* GLWindow::getNativeWindow() const
	{
		return mWindow;
	}


	VkFormat GLWindow::getDepthFormat() const
	{
		return mRenderService->getDepthFormat();
	}

	// Returns the backbuffer
	const BackbufferRenderTarget& GLWindow::getBackbuffer() const
	{
		return mBackbuffer;
	}


	BackbufferRenderTarget& GLWindow::getBackbuffer()
	{
		return mBackbuffer;
	}

	// Set window title
	void GLWindow::setTitle(const std::string& title)
	{
		SDL::setWindowTitle(mWindow, title);
	}


	// Set window position
	void GLWindow::setPosition(const glm::ivec2& position)
	{
		SDL::setWindowPosition(mWindow, position);
	}


	// Set window size 
	void GLWindow::setSize(const glm::ivec2& size)
	{
		// Set set of window
		SDL::setWindowSize(mWindow, size);
        
        // Backbuffer can have more pixels than the represented window (OSX / Retina)
        // Get pixel size accordingly
        mBackbuffer.setSize(SDL::getDrawableWindowSize(mWindow));
	}


	// Get the window size
	const glm::ivec2 GLWindow::getSize() const
	{
		return SDL::getWindowSize(mWindow);
	}


	// Makes the window go full screen
	void GLWindow::setFullScreen(bool value)
	{
		SDL::setFullscreen(mWindow, value);
	}


	// Show window
	void GLWindow::showWindow()
	{
		SDL::showWindow(mWindow, true);
		SDL::raiseWindow(mWindow);
	}


	// Hide window
	void GLWindow::hideWindow()
	{
		SDL::showWindow(mWindow, false);
	}


	// Make this window's context current 
	VkCommandBuffer GLWindow::makeCurrent()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();

		glm::ivec2 window_size = SDL::getWindowSize(mWindow);
		uint32_t window_state = SDL_GetWindowFlags(mWindow);

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

	void GLWindow::beginRenderPass()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();

		glm::ivec2 window_size = SDL::getWindowSize(mWindow);
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapChainFramebuffers[mCurrentImageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (uint32_t)window_size.x, (uint32_t)window_size.y };

		glm::vec4 clearColor = mBackbuffer.getClearColor();
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { clearColor.r, clearColor.g, clearColor.b, clearColor.a };
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

	void GLWindow::endRenderPass()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();
		vkCmdEndRenderPass(mCommandBuffers[current_frame]);
	}

	// Swap buffers
	void GLWindow::swap()
	{
		int	current_frame = mRenderService->getCurrentFrameIndex();
		VkCommandBuffer commandBuffer = mCommandBuffers[current_frame];

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

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

		VkResult result = vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
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



	// The window number
	nap::uint32 GLWindow::getNumber() const
	{
		return SDL::getWindowId(getNativeWindow());
	}


	glm::ivec2 GLWindow::getPosition()
	{
		return SDL::getWindowPosition(mWindow);
	}


	VkSampleCountFlagBits GLWindow::getSampleCount() const
	{
		return mRasterizationSamples;
	}


	bool GLWindow::getSampleShadingEnabled() const
	{
		return mSampleShadingEnabled;
	}
}

RTTI_DEFINE_BASE(nap::GLWindow)
