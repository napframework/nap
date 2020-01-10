// Local includes
#include "glwindow.h"
#include "SDL_vulkan.h"
#include "vulkan/vulkan.h"

// External includes
#include <utility/errorstate.h>
#include <nglutils.h>
#include "renderer.h"

namespace nap
{
	/**
	* createWindow
	*
	* Creates a new opengl window using the parameters specified
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

		if (!errorState.check(new_window != nullptr, "Failed to create window: %s", opengl::getSDLError().c_str()))
			return nullptr;

		return new_window;
	}

	/**
	*	Creates the vulkan surface that is rendered to by the device using SDL
	*/
	static bool createSurface(SDL_Window* window, VkInstance instance, VkPhysicalDevice gpu, uint32_t graphicsFamilyQueueIndex, VkSurfaceKHR& outSurface)
	{
		if (!SDL_Vulkan_CreateSurface(window, instance, &outSurface))
		{
			std::cout << "Unable to create Vulkan compatible surface using SDL\n";
			return false;
		}

		// Make sure the surface is compatible with the queue family and gpu
		VkBool32 supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, graphicsFamilyQueueIndex, outSurface, &supported);
		if (!supported)
		{
			std::cout << "Surface is not supported by physical device!\n";
			return false;
		}

		return true;
	}

	/**
	* Obtain the surface properties that are required for the creation of the swap chain
	*/
	bool getSurfaceProperties(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities) != VK_SUCCESS)
		{
			std::cout << "unable to acquire surface capabilities\n";
			return false;
		}
		return true;
	}

	/**
	* @return if the present modes could be queried and ioMode is set
	* @param outMode the mode that is requested, will contain FIFO when requested mode is not available
	*/
	bool getPresentationMode(VkSurfaceKHR surface, VkPhysicalDevice device, VkPresentModeKHR& ioMode)
	{
		uint32_t mode_count(0);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, NULL) != VK_SUCCESS)
		{
			std::cout << "unable to query present mode count for physical device\n";
			return false;
		}

		std::vector<VkPresentModeKHR> available_modes(mode_count);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_count, available_modes.data()) != VK_SUCCESS)
		{
			std::cout << "unable to query the various present modes for physical device\n";
			return false;
		}

		for (auto& mode : available_modes)
		{
			if (mode == ioMode)
				return true;
		}
		std::cout << "unable to obtain preferred display mode, fallback to FIFO\n";
		ioMode = VK_PRESENT_MODE_FIFO_KHR;
		return true;
	}

	/**
	* Figure out the number of images that are used by the swapchain and
	* available to us in the application, based on the minimum amount of necessary images
	* provided by the capabilities struct.
	*/
	unsigned int getNumberOfSwapImages(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		unsigned int number = capabilities.minImageCount + 1;
		return number > capabilities.maxImageCount ? capabilities.minImageCount : number;
	}

	/**
	*	Returns the size of a swapchain image based on the current surface
	*/
	VkExtent2D getSwapImageSize(glm::ivec2 windowSize, const VkSurfaceCapabilitiesKHR& capabilities)
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
	bool getImageUsage(const VkSurfaceCapabilitiesKHR& capabilities, VkImageUsageFlags& outUsage)
	{
		const std::vector<VkImageUsageFlags>& desired_usages { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
		assert(desired_usages.size() > 0);

		// Needs to be always present
		outUsage = desired_usages[0];

		for (const auto& desired_usage : desired_usages)
		{
			VkImageUsageFlags image_usage = desired_usage & capabilities.supportedUsageFlags;
			if (image_usage != desired_usage)
			{
				std::cout << "unsupported image usage flag: " << desired_usage << "\n";
				return false;
			}

			// Add bit if found as supported color
			outUsage = (outUsage | desired_usage);
		}

		return true;
	}


	/**
	* @return transform based on global declared above, current transform if that transform isn't available
	*/
	VkSurfaceTransformFlagBitsKHR getTransform(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

		std::cout << "unsupported surface transform: " << VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		return capabilities.currentTransform;
	}

	/**
	* @return the most appropriate color space based on the globals provided above
	*/
	bool getFormat(VkPhysicalDevice device, VkSurfaceKHR surface, VkSurfaceFormatKHR& outFormat)
	{
		unsigned int count(0);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr) != VK_SUCCESS)
		{
			std::cout << "unable to query number of supported surface formats";
			return false;
		}

		std::vector<VkSurfaceFormatKHR> found_formats(count);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, found_formats.data()) != VK_SUCCESS)
		{
			std::cout << "unable to query all supported surface formats\n";
			return false;
		}

		// This means there are no restrictions on the supported format.
		// Preference would work
		if (found_formats.size() == 1 && found_formats[0].format == VK_FORMAT_UNDEFINED)
		{
			outFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
			outFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			return true;
		}

		// Otherwise check if both are supported
		for (const auto& found_format_outer : found_formats)
		{
			// Format found
			if (found_format_outer.format == VK_FORMAT_B8G8R8A8_SRGB)		// TODO: we expect this should be VK_FORMAT_B8G8R8A8_SRGB, but it appears that the previous OpenGl implementation didn't use it, so some colors could be washed out
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
				std::cout << "warning: no matching color space found, picking first available one\n!";
				outFormat.colorSpace = found_formats[0].colorSpace;
				return true;
			}
		}

		// No matching formats found
		std::cout << "warning: no matching color format found, picking first available one\n";
		outFormat = found_formats[0];
		return true;
	}

	/**
	* creates the swap chain using utility functions above to retrieve swap chain properties
	* Swap chain is associated with a single window (surface) and allows us to display images to screen
	*/
	static bool createSwapChain(glm::ivec2 windowSize, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, VkSwapchainKHR& outSwapChain, VkExtent2D& outSwapChainExtent, VkFormat& outSwapChainFormat)
	{
		// Get properties of surface, necessary for creation of swap-chain
		VkSurfaceCapabilitiesKHR surface_properties;
		if (!getSurfaceProperties(physicalDevice, surface, surface_properties))
			return false;

		// Get the image presentation mode (synced, immediate etc.)
		VkPresentModeKHR presentation_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		if (!getPresentationMode(surface, physicalDevice, presentation_mode))
			return false;

		// Get other swap chain related features
		unsigned int swap_image_count = getNumberOfSwapImages(surface_properties);

		// Size of the images
		outSwapChainExtent = getSwapImageSize(windowSize, surface_properties);

		// Get image usage (color etc.)
		VkImageUsageFlags usage_flags;
		if (!getImageUsage(surface_properties, usage_flags))
			return false;

		// Get the transform, falls back on current transform when transform is not supported
		VkSurfaceTransformFlagBitsKHR transform = getTransform(surface_properties);

		// Get swapchain image format
		VkSurfaceFormatKHR image_format;
		if (!getFormat(physicalDevice, surface, image_format))
			return false;

		// Old swap chain
		VkSwapchainKHR old_swap_chain = outSwapChain;

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
		swap_info.presentMode = presentation_mode;
		swap_info.clipped = true;
		swap_info.oldSwapchain = NULL;
		swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		// Destroy old swap chain
		if (old_swap_chain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, old_swap_chain, nullptr);
			old_swap_chain = VK_NULL_HANDLE;
		}

		// Create new one
		if (vkCreateSwapchainKHR(device, &swap_info, nullptr, &old_swap_chain) != VK_SUCCESS)
		{
			std::cout << "unable to create swap chain\n";
			return false;
		}

		// Store handle
		outSwapChain = old_swap_chain;
		outSwapChainFormat = image_format.format;
		return true;
	}

	/**
	*	Returns the handles of all the images in a swap chain, result is stored in outImageHandles
	*/
	bool getSwapChainImageHandles(VkDevice device, VkSwapchainKHR chain, std::vector<VkImage>& outImageHandles)
	{
		unsigned int image_count(0);
		VkResult res = vkGetSwapchainImagesKHR(device, chain, &image_count, nullptr);
		if (res != VK_SUCCESS)
		{
			std::cout << "unable to get number of images in swap chain\n";
			return false;
		}

		outImageHandles.clear();
		outImageHandles.resize(image_count);
		if (vkGetSwapchainImagesKHR(device, chain, &image_count, outImageHandles.data()) != VK_SUCCESS)
		{
			std::cout << "unable to get image handles from swap chain\n";
			return false;
		}
		return true;
	}

	static bool createRenderPass(VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat, VkRenderPass& renderPass)
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChainImageFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		return vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS;
	}

	static bool createImageViews(VkDevice device, std::vector<VkImageView>& swapChainImageViews, const std::vector<VkImage>& swapChainImages, VkFormat swapChainFormat)
	{
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) 
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}

	static bool createFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers, std::vector<VkImageView>& swapChainImageViews, VkImageView depthImageView, VkRenderPass renderPass, VkExtent2D extent)
	{
		framebuffers.resize(swapChainImageViews.size());

		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{

			std::array<VkImageView, 2> attachments = {
				swapChainImageViews[i],
				depthImageView
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
				return false;
		}

		return true;
	}


	static bool createCommandBuffers(VkDevice device, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers, int inNumCommandBuffers) 
	{
		commandBuffers.resize(inNumCommandBuffers);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		return vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) == VK_SUCCESS;
	}

	bool createSyncObjects(VkDevice device, std::vector<VkSemaphore>& imageAvailableSemaphores, std::vector<VkSemaphore>& renderFinishedSemaphores, std::vector<VkFence>& inFlightFences, int numFramesInFlight)
	{
		imageAvailableSemaphores.resize(numFramesInFlight);
		renderFinishedSemaphores.resize(numFramesInFlight);
		inFlightFences.resize(numFramesInFlight);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < numFramesInFlight; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
			{
				return false;
			}
		}

		return true;
	}

	unsigned int findPresentFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface)
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

	static bool createImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			return false;

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		uint32_t mem_type_index = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);
		if (mem_type_index == -1)
			return false;

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = mem_type_index;

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
			return false;

		vkBindImageMemory(device, image, imageMemory, 0);
		return true;
	}

	bool createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView) 
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
			return false;

		return true;
	}

	static bool createDepthBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkExtent2D swapchainExtent, VkFormat depthFormat, VkImage& depthImage, VkDeviceMemory& depthImageMemory, VkImageView& depthImageView)
	{
		if (!createImage(physicalDevice, device, swapchainExtent.width, swapchainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory))
			return false;

		if (!createImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, depthImageView))
			return false;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	// GL Render window constructor
	GLWindow::GLWindow() { }


	GLWindow::~GLWindow()
	{	
		if (mWindow != nullptr)
			SDL_DestroyWindow(mWindow);
	}


	void GLWindow::applySettings(const RenderWindowSettings& settings)
	{
		setSize(glm::vec2(settings.width, settings.height));
		opengl::setWindowPosition(mWindow, glm::vec2(settings.x, settings.y));
		opengl::setWindowResizable(mWindow, settings.resizable);
		opengl::setWindowBordered(mWindow, !settings.borderless);
		opengl::setWindowTitle(mWindow, settings.title);
		if (settings.visible)
			showWindow();
		else
			hideWindow();
	}


	// Creates a window with an associated OpenGL context
	bool GLWindow::init(const RenderWindowSettings& settings, Renderer& renderer, nap::utility::ErrorState& errorState)
	{
		mDevice = renderer.getDevice();
		mGraphicsQueue = renderer.getGraphicsQueue();

		// create the window
		mWindow = createSDLWindow(settings, errorState);
		if (mWindow == nullptr)
			return false;

		setSize(glm::vec2(settings.width, settings.height));

		VkPhysicalDevice physicalDevice = renderer.getPhysicalDevice();

		if (!createSurface(mWindow, renderer.getVulkanInstance(), physicalDevice, renderer.getGraphicsQueueIndex(), mSurface))
			return false;

		VkFormat swapchainFormat;
		VkExtent2D swapchainExtent;
		if (!createSwapChain(getSize(), mSurface, physicalDevice, mDevice, mSwapchain, swapchainExtent, swapchainFormat))
			return false;

		// Get image handles from swap chain
		std::vector<VkImage> chain_images;
		if (!getSwapChainImageHandles(mDevice, mSwapchain, chain_images))
			return false;

		if (!createRenderPass(mDevice, swapchainFormat, renderer.getDepthFormat(), mRenderPass))
			return false;

		if (!createImageViews(mDevice, mSwapChainImageViews, chain_images, swapchainFormat))
			return false;

		if (!createDepthBuffer(physicalDevice, mDevice, swapchainExtent, renderer.getDepthFormat(), mDepthImage, mDepthImageMemory, mDepthImageView))
			return false;

		if (!createFramebuffers(mDevice, mSwapChainFramebuffers, mSwapChainImageViews, mDepthImageView, mRenderPass, swapchainExtent))
			return false;

		const int maxFramesInFlight = 2;
		if (!createCommandBuffers(mDevice, renderer.getCommandPool(), mCommandBuffers, maxFramesInFlight))
			return false;

		if (!createSyncObjects(mDevice, mImageAvailableSemaphores, mRenderFinishedSemaphores, mInFlightFences, maxFramesInFlight))
			return false;

		unsigned int presentQueueIndex = findPresentFamilyIndex(physicalDevice, mSurface);
		if (presentQueueIndex == -1)
			return false;

		vkGetDeviceQueue(mDevice, presentQueueIndex, 0, &mPresentQueue);

		return true;
	}


	// Returns the actual opengl window
	SDL_Window* GLWindow::getNativeWindow() const
	{
		return mWindow;
	}


	// Returns the backbuffer
	const opengl::BackbufferRenderTarget& GLWindow::getBackbuffer() const
	{
		return mBackbuffer;
	}


	opengl::BackbufferRenderTarget& GLWindow::getBackbuffer()
	{
		return mBackbuffer;
	}

	// Set window title
	void GLWindow::setTitle(const std::string& title)
	{
		opengl::setWindowTitle(mWindow, title);
	}


	// Set opengl window position
	void GLWindow::setPosition(const glm::ivec2& position)
	{
		opengl::setWindowPosition(mWindow, position);
	}


	// Set opengl window size 
	void GLWindow::setSize(const glm::ivec2& size)
	{
		// Set set of window
		opengl::setWindowSize(mWindow, size);
        
        // Backbuffer can have more pixels than the represented window (OSX / Retina)
        // Get pixel size accordingly
        mBackbuffer.setSize(opengl::getDrawableWindowSize(mWindow));
	}


	// Get the window size
	const glm::ivec2 GLWindow::getSize() const
	{
		return opengl::getWindowSize(mWindow);
	}


	// Makes the window go full screen
	void GLWindow::setFullScreen(bool value)
	{
		opengl::setFullscreen(mWindow, value);
	}


	// Show opengl window
	void GLWindow::showWindow()
	{
		opengl::showWindow(mWindow, true);
		opengl::raiseWindow(mWindow);
	}


	// Hide opengl window
	void GLWindow::hideWindow()
	{
		opengl::showWindow(mWindow, false);
	}

	VkCommandBuffer	GLWindow::getCommandBuffer()
	{
		return mCommandBuffers[mCurrentFrame];
	}

	// Make this window's context current 
	void GLWindow::makeCurrent()
	{
		vkWaitForFences(mDevice, 1, &mInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(mDevice, 1, &mInFlightFences[mCurrentFrame]);

		vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &mCurrentImageIndex);

		VkCommandBuffer commandBuffer = mCommandBuffers[mCurrentFrame];
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		glm::ivec2 window_size = getSize();

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

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}


	// Swap OpenGL buffers
	void GLWindow::swap()
	{
		VkCommandBuffer commandBuffer = mCommandBuffers[mCurrentFrame];

		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffers[mCurrentFrame];

		VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFences[mCurrentFrame]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { mSwapchain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &mCurrentImageIndex;

		vkQueuePresentKHR(mPresentQueue, &presentInfo);

		mCurrentFrame = (mCurrentFrame + 1) % mCommandBuffers.size();
	}



	// The window number
	nap::uint32 GLWindow::getNumber() const
	{
		return opengl::getWindowId(getNativeWindow());
	}


	glm::ivec2 GLWindow::getPosition()
	{
		return opengl::getWindowPosition(mWindow);
	}

}

RTTI_DEFINE_BASE(nap::GLWindow)
