/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "renderwindow.h"
#include "sdlhelpers.h"
#include "renderutils.h"
#include "imagedata.h"

#include <renderservice.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <SDL_vulkan.h>
#include <SDL_hints.h>
#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::RenderWindow::EPresentationMode)
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::Immediate,	"Immediate"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::Mailbox,		"Mailbox"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::FIFO_Relaxed,	"FIFO Relaxed"),
	RTTI_ENUM_VALUE(nap::RenderWindow::EPresentationMode::FIFO,			"FIFO")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderWindow, "Desktop render window")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Borderless",				&nap::RenderWindow::mBorderless,		nap::rtti::EPropertyMetaData::Default,	"If the window has borders")
	RTTI_PROPERTY("Resizable",				&nap::RenderWindow::mResizable,			nap::rtti::EPropertyMetaData::Default,	"If the window is resizable")
	RTTI_PROPERTY("Visible",				&nap::RenderWindow::mVisible,			nap::rtti::EPropertyMetaData::Default,	"If the window is visible")
	RTTI_PROPERTY("AlwaysOnTop",			&nap::RenderWindow::mAlwaysOnTop,		nap::rtti::EPropertyMetaData::Default,	"Bring the window to the front and keep it above the rest")
	RTTI_PROPERTY("SampleShading",			&nap::RenderWindow::mSampleShading,		nap::rtti::EPropertyMetaData::Default,	"Reduces texture aliasing at higher computational cost")
	RTTI_PROPERTY("Title",					&nap::RenderWindow::mTitle,				nap::rtti::EPropertyMetaData::Default,	"Window display title")
	RTTI_PROPERTY("Width",					&nap::RenderWindow::mWidth,				nap::rtti::EPropertyMetaData::Default,	"Horizontal resolution")
	RTTI_PROPERTY("Height",					&nap::RenderWindow::mHeight,			nap::rtti::EPropertyMetaData::Default,	"Vertical resolution")
	RTTI_PROPERTY("Mode",					&nap::RenderWindow::mMode,				nap::rtti::EPropertyMetaData::Default,	"Image presentation mode: 'Immediate' turns VSync off, the others turn VSync on")
	RTTI_PROPERTY("ClearColor",				&nap::RenderWindow::mClearColor,		nap::rtti::EPropertyMetaData::Default,	"Initial window clear color")
	RTTI_PROPERTY("Samples",				&nap::RenderWindow::mRequestedSamples,	nap::rtti::EPropertyMetaData::Default,	"The number of MSAA samples to use")
	RTTI_PROPERTY("AdditionalSwapImages",	&nap::RenderWindow::mAddedSwapImages,	nap::rtti::EPropertyMetaData::Default,	"Number of additional swapchain images to create")
	RTTI_PROPERTY("Clear",					&nap::RenderWindow::mClear,				nap::rtti::EPropertyMetaData::Default,	"Whether to clear the render window at the start of each render pass")
	RTTI_PROPERTY("RestoreSize",			&nap::RenderWindow::mRestoreSize,		nap::rtti::EPropertyMetaData::Default,	"If window size is restored from the previous run")
	RTTI_PROPERTY("RestorePosition",		&nap::RenderWindow::mRestorePosition,	nap::rtti::EPropertyMetaData::Default,	"If window position is restored from the previous run")
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static SDL Functions
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Creates a new SDL window based on the settings provided by the render window.
	 * This call should only be used to create a top level, non embedded, window.
	 * @return: the create window, nullptr if not successful
	 */
	static SDL_Window* createSDLWindow(const RenderWindow& renderWindow, utility::ErrorState& error)
	{
		// Construct options
		Uint32 options = SDL_WINDOW_VULKAN;
		options |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
		options |= renderWindow.mResizable  ? SDL_WINDOW_RESIZABLE  : 0x0U;
		options |= renderWindow.mBorderless ? SDL_WINDOW_BORDERLESS : 0x0U;
		options |= renderWindow.mAlwaysOnTop ? SDL_WINDOW_ALWAYS_ON_TOP : 0x0U;

		// Always hide window until added and configured by render service
		options |= SDL_WINDOW_HIDDEN;

		// Create window
		// TODO: Where is the center option?
		SDL_Window* new_window = SDL_CreateWindow(renderWindow.mTitle.c_str(),
			renderWindow.mWidth,
			renderWindow.mHeight,
			options);

		if (!error.check(new_window != nullptr, "Failed to create window: %s", SDL::getSDLError().c_str()))
			return nullptr;

		return new_window;
	}


	//////////////////////////////////////////////////////////////////////////
	// Static Vulkan Functions
	//////////////////////////////////////////////////////////////////////////


	/**
	 *	Creates the vulkan surface that is rendered to by the device using SDL
	 */
	static bool createSurface(SDL_Window* window, VkInstance instance, VkSurfaceKHR& outSurface, utility::ErrorState& errorState)
	{
		// Use SDL to create the surface
		// TODO: Pass our own allocator instead of using system default
		return errorState.check(SDL_Vulkan_CreateSurface(window, instance, NULL, &outSurface),
			"Unable to create Vulkan compatible surface using SDL");
	}


	/**
	 * Returns name of swapchain mode
	 */
	static std::string getPresentModeName(VkPresentModeKHR presentMode)
	{
		switch (presentMode)
		{
		case VK_PRESENT_MODE_FIFO_KHR:
			return "FIFO";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			return "FIFO Relaxed";
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			return "Immediate";
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return "Mailbox";
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			return "Shared Continuous Refresh";
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			return "Shared Demand Refresh";
		default:
			return "Unknown";
		}
	}


	/**
	 * @return the Vulkan image presentation mode for the given NAP mode
	 */
	static VkPresentModeKHR getPresentMode(RenderWindow::EPresentationMode presentationMode)
	{
		switch (presentationMode)
		{
		case RenderWindow::EPresentationMode::FIFO:
			return VK_PRESENT_MODE_FIFO_KHR;
		case RenderWindow::EPresentationMode::FIFO_Relaxed:
			return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		case RenderWindow::EPresentationMode::Immediate:
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		case RenderWindow::EPresentationMode::Mailbox:
			return VK_PRESENT_MODE_MAILBOX_KHR;
		default:
			assert(false);
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}


	/**
	 * Returns if the requested presentation mode is supported, fall-back = FIFO_KHR
	 */
	static bool findCompatiblePresentMode(VkSurfaceKHR surface, VkPhysicalDevice device, VkPresentModeKHR requestedMode, VkPresentModeKHR& outMode, utility::ErrorState& errorState)
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
		uint32 count(0);
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
			if (found_format_outer.format == VK_FORMAT_B8G8R8A8_UNORM)
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
	static bool createSwapChain(VkPresentModeKHR presentMode, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32 swapImageCount, const VkSurfaceCapabilitiesKHR& surfaceCapabilities, VkExtent2D extent, VkSwapchainKHR& outSwapChain, VkFormat& outSwapChainFormat, utility::ErrorState& errorState)
	{
		// Get image usage (color etc.)
		VkImageUsageFlags usage_flags;
		if (!getImageUsage(surfaceCapabilities, usage_flags, errorState))
			return false;

		// Get the transform, falls back on current transform when transform is not supported
		VkSurfaceTransformFlagBitsKHR transform = getTransform(surfaceCapabilities);

		// Get swapchain image format
		VkSurfaceFormatKHR image_format;
		if (!getFormat(physicalDevice, surface, image_format, errorState))
			return false;

		// Sharing mode = exclusive, graphics and presentation queue must be the same. Sharing not supported.
		VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;

		// Ensure extent is valid
		if (!errorState.check(extent.width > 0 && extent.height > 0, "Image extent members 'width' and 'height' must be higher than 0"))
			return false;

		// Populate swapchain creation info
		VkSwapchainCreateInfoKHR swap_info = {};
		swap_info.pNext = nullptr;
		swap_info.flags = 0;
		swap_info.surface = surface;
		swap_info.minImageCount = swapImageCount;
		swap_info.imageFormat = image_format.format;
		swap_info.imageColorSpace = image_format.colorSpace;
		swap_info.imageExtent = extent;
		swap_info.imageArrayLayers = 1;
		swap_info.imageUsage = usage_flags;
		swap_info.imageSharingMode = sharing_mode;
		swap_info.queueFamilyIndexCount = 0;
		swap_info.pQueueFamilyIndices = nullptr;
		swap_info.preTransform = transform;
		swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swap_info.presentMode = presentMode;
		swap_info.clipped = true;
		swap_info.oldSwapchain = VK_NULL_HANDLE;
		swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

		// Create new one
		if (!errorState.check(vkCreateSwapchainKHR(device, &swap_info, nullptr, &outSwapChain) == VK_SUCCESS, "Unable to create swap chain"))
			return false;

		// Successfully created swapchain, store format and return
		outSwapChainFormat = image_format.format;
		return true;
	}


	/**
	 * Returns the handles of all the images in a swap chain, result is stored in outImageHandles
	 */
	static bool getSwapChainImageHandles(VkDevice device, VkSwapchainKHR chain, std::vector<VkImage>& outImageHandles, utility::ErrorState& errorState)
	{
		uint32 image_count(0);
		if (!errorState.check(vkGetSwapchainImagesKHR(device, chain, &image_count, nullptr) == VK_SUCCESS, "Unable to get number of images in swap chain"))
			return false;

		outImageHandles.clear();
		outImageHandles.resize(image_count);
		if (!errorState.check(vkGetSwapchainImagesKHR(device, chain, &image_count, outImageHandles.data()) == VK_SUCCESS, "Unable to get image handles from swap chain"))
			return false;

		return true;
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


	static bool createFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers, VkImageView colorImageView, VkImageView depthImageView, std::vector<VkImageView>& swapChainImageViews, VkRenderPass renderPass, VkExtent2D extent, VkSampleCountFlagBits samples, utility::ErrorState& errorState)
	{
		// Create a frame buffer for every view in the swapchain.
		framebuffers.resize(swapChainImageViews.size(), VK_NULL_HANDLE);
		for (size_t i = 0; i < swapChainImageViews.size(); i++)
		{
			std::array<VkImageView, 3> attachments = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
			if (samples == VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT)
			{
				attachments[0] = swapChainImageViews[i];
				attachments[1] = depthImageView;
			}
			else
			{
				attachments[0] = colorImageView;
				attachments[1] = depthImageView;
				attachments[2] = swapChainImageViews[i];
			}

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = samples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3;
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (!errorState.check(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
				return false;
		}
		return true;
	}


	static bool createCommandBuffers(VkDevice device, VkCommandPool commandPool, std::vector<VkCommandBuffer>& commandBuffers, int count, utility::ErrorState& errorState)
	{
		commandBuffers.resize(count, VK_NULL_HANDLE);
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		return errorState.check(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) == VK_SUCCESS, "Failed to allocate command buffers");
	}


	static bool createSyncPrimitives(VkDevice device, std::vector<VkSemaphore>& semaphores, int count, utility::ErrorState& errorState)
	{
		semaphores.resize(count, VK_NULL_HANDLE);
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (auto& handle : semaphores)
		{
			if (!errorState.check(vkCreateSemaphore(device, &info, nullptr, &handle) == VK_SUCCESS, "Failed to create image sync primitives"))
				return false;
		}
		return true;
	}


	static int canPresent(VkPhysicalDevice device, uint32 graphicsQueueIndex, VkSurfaceKHR surface, utility::ErrorState& error)
	{
		// First check if the graphics queue supports presentation
		VkBool32 supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, graphicsQueueIndex, surface, &supported);
		return error.check(supported > 0, "The selected graphics queue %d does not support presenting a swapchain image.", graphicsQueueIndex);
	}


	static bool createColorResource(const RenderService& renderer, VkExtent2D swapchainExtent, VkFormat colorFormat, VkSampleCountFlagBits sampleCount, ImageData& outData, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), swapchainExtent.width, swapchainExtent.height, colorFormat, 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outData.mImage, outData.mAllocation, outData.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outData.getImage(), colorFormat, 1, VK_IMAGE_ASPECT_COLOR_BIT, outData.mView, errorState))
			return false;

		return true;
	}


	static bool createDepthResource(const RenderService& renderer, VkExtent2D swapchainExtent, VkSampleCountFlagBits sampleCount, ImageData& outImage, utility::ErrorState& errorState)
	{
		if (!create2DImage(renderer.getVulkanAllocator(), swapchainExtent.width, swapchainExtent.height, renderer.getDepthFormat(), 1, sampleCount, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, outImage.mImage, outImage.mAllocation, outImage.mAllocationInfo, errorState))
			return false;

		if (!create2DImageView(renderer.getDevice(), outImage.getImage(), renderer.getDepthFormat(), 1, VK_IMAGE_ASPECT_DEPTH_BIT, outImage.mView, errorState))
			return false;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Member Functions
	//////////////////////////////////////////////////////////////////////////


	RenderWindow::RenderWindow(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	RenderWindow::RenderWindow(Core& core, SDL_Window* windowHandle) :
		mRenderService(core.getService<RenderService>()), mExternalHandle(windowHandle)
	{ }


	RenderWindow::~RenderWindow()
	{
		// Return immediately if there's no actual native window present.
		// This is the case when the window is created but not initialized or did not initialize properly.
		if (mSDLWindow == nullptr)
			return;

		// Wait for device to go idle before destroying the window-related resources
		if (mDevice != VK_NULL_HANDLE)
		{
			VkResult result = vkDeviceWaitIdle(mDevice);
			assert(result == VK_SUCCESS);
		}

		// Destroy resources associated with swapchain
		destroySwapChainResources();

		// Destroy window surface
		if (mSurface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(mRenderService->getVulkanInstance(), mSurface, nullptr);
			mSurface = VK_NULL_HANDLE;
		}

		// Destroy SDL Window
		SDL_DestroyWindow(mSDLWindow);
		mSDLWindow = nullptr;
	}


	bool RenderWindow::init(utility::ErrorState& errorState)
	{
		// Forbid creation of window when headless rendering is enabled
		// Window might be created, when display device is attached, but surface creation will definately fail.
		if (!errorState.check(!mRenderService->isHeadless(), "Can't create window, headless rendering is enabled"))
			return false;

		// Create new top-level SDL window if there is no external handle available.
		// An external handle is provided by an external environment, when the window is embedded.
		// Standalone nap applications have no handle, applets generally do.
		assert(mSDLWindow == nullptr);
		mSDLWindow = mExternalHandle != nullptr ? mExternalHandle :
			createSDLWindow(*this, errorState);

		// Ensure window is valid
		if (mSDLWindow == nullptr)
			return false;

		// Fetch required vulkan handles
		mDevice = mRenderService->getDevice();

		// Acquire max number of MSAA samples, issue warning if requested number of samples is not obtained
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

		// Create render surface for window
		VkPhysicalDevice pyshical_device = mRenderService->getPhysicalDevice();
		if (!createSurface(mSDLWindow, mRenderService->getVulkanInstance(), mSurface, errorState))
			return false;

		// Get compatible Vulkan presentation mode
		VkPresentModeKHR req_present_mode = getPresentMode(mMode);
		if (!findCompatiblePresentMode(mSurface, pyshical_device, req_present_mode, mPresentationMode, errorState))
			return false;

		// Check if the selected presentation mode matches our request
		if (req_present_mode != mPresentationMode)
			nap::Logger::warn("%s: Unsupported presentation mode: %s, switched to: %s", mID.c_str(), getPresentModeName(req_present_mode).c_str(), getPresentModeName(mPresentationMode).c_str());

		// Ensure we can present a swapchain image using the selected render service queue index.
		if (!canPresent(pyshical_device, mRenderService->getQueueIndex(), mSurface, errorState))
			return false;

		// Get presentation queue
		vkGetDeviceQueue(mDevice, mRenderService->getQueueIndex(), 0, &mPresentQueue);

		// Update surface capabilities
		if (!getSurfaceCapabilities(errorState))
			return false;

		// Create swapchain based on current window properties
		if (!createSwapChainResources(errorState))
			return false;

		// Add window to render service
		mRenderService->addWindow(*this);

		// Show if requested
		if (mVisible)
			show();

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


	void RenderWindow::setTitle(std::string newTitle)
	{
		SDL::setWindowTitle(mSDLWindow, newTitle);
	}


	void RenderWindow::setFullscreen(bool value)
	{
		if(!SDL::setFullscreen(mSDLWindow, value))
			nap::Logger::error(SDL::getSDLError());
	}


	void RenderWindow::toggleFullscreen()
	{
		bool cur_state = SDL::getFullscreen(mSDLWindow);
		if(!SDL::setFullscreen(mSDLWindow, !cur_state))
			nap::Logger::error(SDL::getSDLError());
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
		if (size != SDL::getWindowSize(mSDLWindow))
		{
			SDL::setWindowSize(mSDLWindow, size);
		}
	}


	const glm::ivec2 RenderWindow::getBufferSize() const
	{
		return SDL::getDrawableWindowSize(mSDLWindow);
	}


	const glm::ivec2 RenderWindow::getSize() const
	{
		return SDL::getWindowSize(mSDLWindow);
	}


	void RenderWindow::setClearColor(const RGBAColorFloat& color)
	{
		mClearColor = color;
	}


	const RGBAColorFloat& RenderWindow::getClearColor() const
	{
		return mClearColor;
	}


	void RenderWindow::setPosition(const glm::ivec2& position) const
	{
		if (!SDL::setWindowPosition(mSDLWindow, position))
		{
			Logger::error("Unable to set '%s' position: '%s'",
				mID.c_str(), SDL::getSDLError().c_str());
		}
	}


	glm::ivec2 RenderWindow::getPosition() const
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


	VkCommandBuffer RenderWindow::beginRecording()
	{
		// Recreate the entire swapchain when the framebuffer (size or format) no longer matches the existing swapchain .
		// This occurs when vkAcquireNextImageKHR or vkQueuePresentKHR  signals that the image is out of date.
		if (mRecreateSwapchain)
		{
 			utility::ErrorState errorState;
			if (!recreateSwapChain(errorState))
			{
				Logger::fatal("Unable to recreate swapchain: %s", errorState.toString().c_str());
				assert(false);
			}
		}

		// The swapchain extent can have a valid (higher than zero) size when the window is minimized.
		// However, Vulkan internally knows this is not the case (it sees it as a zero-sized window), which will result in 
		// errors being thrown by vkAcquireNextImageKHR etc if we try to render anyway. So, to workaround this issue, we also consider minimized windows to be of zero size.
		// In either case, when the window is not visible, we can't render to it since there is no valid swap chain. So, we return a nullptr to signal this to the client.
		if (!validSwapchainExtent() || isMinimized())
			return VK_NULL_HANDLE;

		// Under a wayland session, 'vkAcquireNextImageKHR' doesn't fail when the active swap chain extent doesn't match the surface.
		// This path ensures that, under wayland, the swapchain is re-created when the buffer / swap delta exceeds 2 texels.
		// I don't like this to be the default, because the `vkAcquireNextImageKHR` should  inform us about this.
		if (mRenderService->getVideoDriver() == EVideoDriver::Wayland)
		{
			auto diff = glm::abs(getBufferSize() - glm::ivec2(mSwapchainExtent.width, mSwapchainExtent.height));
			if (diff.x + diff.y > 2)
			{
				mRecreateSwapchain = true;
				return VK_NULL_HANDLE;
			}
		}

		// Figure out which swapchain image to draw to next, the semaphore is triggered when the image becomes available.
		// On which the renderer waits to become available when the queue is submitted in RenderWindow::endRecording().
		int	frame_index = mRenderService->getCurrentFrameIndex(); 
		assert(mSwapchain != VK_NULL_HANDLE);
		assert(frame_index < mAcquireSemaphores.size());
		VkResult result = vkAcquireNextImageKHR(mDevice, mSwapchain, UINT64_MAX, mAcquireSemaphores[frame_index], VK_NULL_HANDLE, &mImageIndex);

		// If the next image is for some reason out of date, recreate the framebuffer the next frame and record nothing.
		// This situation occurs when the swapchain dimensions don't match the current extent, ie: window has been resized.
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			mRecreateSwapchain = true;
			return VK_NULL_HANDLE;
		}

		// We expect to have a working image here, otherwise something is seriously wrong.
		NAP_ASSERT_MSG(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
			"Unable to retrieve the index of the next available presentable image");

		// Reset command buffer for current frame
		VkCommandBuffer commandBuffer = mCommandBuffers[frame_index];
		vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		// Start recording commands
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
		assert(result == VK_SUCCESS);
		return commandBuffer;
	}


	void RenderWindow::endRecording()
	{
		// Stop recording command buffer
		int	frame_index = mRenderService->getCurrentFrameIndex();
		assert(frame_index < mCommandBuffers.size());
		VkCommandBuffer command_buffer = mCommandBuffers[frame_index];
		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) 
			throw std::runtime_error("failed to record command buffer!");

		// GPU needs to wait for the presentation engine to return the image to the swapchain (if still busy), so
		// the GPU will wait for the image available semaphore to be signaled when we start writing to the color attachment.
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &mAcquireSemaphores[frame_index];
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &mCommandBuffers[frame_index];

		// When the command buffer has completed execution, the render finished semaphore is signaled. This semaphore
		// is used by the GPU presentation engine to wait before presenting the finished image to screen.
		assert(mImageIndex < mSubmitSemaphores.size());
		VkSemaphore submit_semaphore = mSubmitSemaphores[mImageIndex];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &submit_semaphore;

		// Submit the queue for rendering
		VkResult result = vkQueueSubmit(mRenderService->getQueue(), 1, &submit_info, VK_NULL_HANDLE);
		assert(result == VK_SUCCESS);

		// Set the rendering bit of queue submit ops of the current frame
		mRenderService->mFramesInFlight[frame_index].mQueueSubmitOps |= RenderService::EQueueSubmitOp::Rendering;

		// Create presentation information -> wait for the render finished semaphore to be signalled before presenting
		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &submit_semaphore;
		present_info.swapchainCount = 1;						// Await only the render finished semaphore
		present_info.pSwapchains = &mSwapchain;
		present_info.pImageIndices = &mImageIndex;

		// According to the spec, vkQueuePresentKHR can return VK_ERROR_OUT_OF_DATE_KHR when the framebuffer no longer matches the swapchain.
		// In our case this should only happen due to window size changes, which is handled in makeCurrent. So, we don't attempt to handle it here.
		// If the window is resized during a render call, the result should be VK_ERROR_OUT_OF_DATE_KHR or VK_SUBOPTIMAL_KHR.
		// If that's the case the swapchain needs to be re-created at the beginning of the recorded frame.
		switch(vkQueuePresentKHR(mPresentQueue, &present_info))
		{
		case VK_SUCCESS:
            break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
			mRecreateSwapchain = true;
			break;
		default:
			assert(false);
			break;
		}
	}


	bool RenderWindow::recreateSwapChain(utility::ErrorState& errorState)
	{
		// Wait to ensure all command buffers have finished processing
		VkResult result = vkDeviceWaitIdle(mRenderService->getDevice());
		if(!errorState.check(result == VK_SUCCESS, "Unable to destroy swap chain resources, device not in idle state"))
			return false;

		// Destroy all swapchain related Vulkan resources
		mRecreateSwapchain = false;
		destroySwapChainResources();

		// Update surface capabilities
		if (!getSurfaceCapabilities(errorState))
			return false;

		// Don't create a swapchain if width or height is invalid. 
		// Vulkan requires the 'imageExtent' members width and height to be non zero.
		if (mSwapchainExtent.width == 0 || mSwapchainExtent.height == 0)
			return true;

		// Create new swapchain based on current window properties
		return createSwapChainResources(errorState);
	}


	bool RenderWindow::createSwapChainResources(utility::ErrorState& errorState)
	{
		// Check if number of requested images is supported based on queried abilities
		// When maxImageCount == 0 there is no theoretical limit, otherwise it has to fall within the range of min-max
		int min_image_count = mSurfaceCapabilities.minImageCount + mAddedSwapImages;
		if (min_image_count > mSurfaceCapabilities.maxImageCount && mSurfaceCapabilities.maxImageCount != 0)
		{
			nap::Logger::warn("%s: Requested number of swap chain images: %d exceeds hardware limit", mID.c_str(), min_image_count, mSurfaceCapabilities.maxImageCount);
			min_image_count = mSurfaceCapabilities.maxImageCount;
		}

		// Create swapchain, allowing us to acquire images to render to.
		if (!createSwapChain(mPresentationMode, mSurface, mRenderService->getPhysicalDevice(), mDevice, min_image_count, mSurfaceCapabilities, mSwapchainExtent, mSwapchain, mSwapchainFormat, errorState))
			return false;

		// Get image handles from swap chain
		std::vector<VkImage> chain_images;
		if (!getSwapChainImageHandles(mDevice, mSwapchain, chain_images, errorState))
			return false;

		// Create image view for every image in swapchain
		nap::Logger::debug("%s: Created swapchain with %d swap chain images", mID.c_str(), chain_images.size());
		if (!createSwapchainImageViews(mDevice, mSwapChainImageViews, chain_images, mSwapchainFormat, errorState))
			return false;

        // Create render pass and resources used by render pass
        // With only 1 multi-sample, don't create a render pass with a resolve step
        if (!createRenderPass(mDevice, mSwapchainFormat, mRenderService->getDepthFormat(), mRasterizationSamples, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, mClear, mRenderPass, errorState))
            return false;

		// Create attachment resources
		switch (mRasterizationSamples)
		{
			case VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT:
			{
				if (!createDepthResource(*mRenderService, mSwapchainExtent, mRasterizationSamples, mDepthImage, errorState))
					return false;
				break;
			}
			default:
			{
				if (!createDepthResource(*mRenderService, mSwapchainExtent, mRasterizationSamples, mDepthImage, errorState))
					return false;

				if (!createColorResource(*mRenderService, mSwapchainExtent, mSwapchainFormat, mRasterizationSamples, mColorImage, errorState))
					return false;
				break;
			}
		}

		// Create swapchain frame-buffers
		if (!createFramebuffers(mDevice, mSwapChainFramebuffers, mColorImage.getView(), mDepthImage.getView(), mSwapChainImageViews, mRenderPass, mSwapchainExtent, mRasterizationSamples, errorState))
			return false;

		// Create command buffers
		if (!createCommandBuffers(mDevice, mRenderService->getCommandPool(), mCommandBuffers, mRenderService->getMaxFramesInFlight(), errorState))
			return false;

		// Create frame sync primitives
		if (!createSyncPrimitives(mDevice, mAcquireSemaphores, mRenderService->getMaxFramesInFlight(), errorState))
			return false;

		// Create queue submit primitives -> same size as swap-chain image count
		if (!createSyncPrimitives(mDevice, mSubmitSemaphores, static_cast<int>(chain_images.size()), errorState))
			return false;

		return true;
	}


	void RenderWindow::destroySwapChainResources()
	{
		// Destroy queue submission semaphores
		for (VkSemaphore semaphore : mSubmitSemaphores)
		{
			if (semaphore != VK_NULL_HANDLE)
				vkDestroySemaphore(mDevice, semaphore, nullptr);
		}
		mSubmitSemaphores.clear();

		// Destroy frame-related semaphores
		for (VkSemaphore semaphore : mAcquireSemaphores)
		{
			if (semaphore != VK_NULL_HANDLE)
				vkDestroySemaphore(mDevice, semaphore, nullptr);
		}
		mAcquireSemaphores.clear();

		// Destroy all frame buffers
		for (VkFramebuffer frame_buffer : mSwapChainFramebuffers)
		{
			if (frame_buffer != VK_NULL_HANDLE)
				vkDestroyFramebuffer(mDevice, frame_buffer, nullptr);
		}
		mSwapChainFramebuffers.clear();

		// Free command buffers
		if (!mCommandBuffers.empty())
		{
			vkFreeCommandBuffers(mDevice, mRenderService->getCommandPool(), static_cast<uint32>(mCommandBuffers.size()), mCommandBuffers.data());
			mCommandBuffers.clear();
		}

		// Destroy render pass if present
		if (mRenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
			mRenderPass = VK_NULL_HANDLE;
		}

		// Destroy swapchain image views if present
		for (VkImageView view : mSwapChainImageViews)
		{
			if (view != VK_NULL_HANDLE)
				vkDestroyImageView(mDevice, view, nullptr);
		}
		mSwapChainImageViews.clear();

		// Destroy depth and color image
		utility::destroyImageAndView(mDepthImage, mDevice, mRenderService->getVulkanAllocator());
		utility::destroyImageAndView(mColorImage, mDevice, mRenderService->getVulkanAllocator());

		// finally, destroy swapchain
		if (mSwapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
			mSwapchain = VK_NULL_HANDLE;
		}
		mSwapchainExtent = { 0,0 };
	}


	bool RenderWindow::validSwapchainExtent() const
	{
		return mSwapchainExtent.width > 0 && mSwapchainExtent.height > 0;
	}


	void RenderWindow::beginRendering()
	{
		// Create information for render pass
		VkRenderPassBeginInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = mRenderPass;
		render_pass_info.framebuffer = mSwapChainFramebuffers[mImageIndex];
		render_pass_info.renderArea.offset = { 0, 0 };
		render_pass_info.renderArea.extent = mSwapchainExtent;

		// Clear color
		std::array<VkClearValue, 3> clear_values = {};
		clear_values[0].color = { mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3] };
		clear_values[1].depthStencil = { 1.0f, 0 };
		clear_values[2].color = { mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3] };
		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		// Begin render pass, using command buffer associated with current frame.
		int	current_frame = mRenderService->getCurrentFrameIndex();
		vkCmdBeginRenderPass(mCommandBuffers[current_frame], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent = mSwapchainExtent;
		vkCmdSetScissor(mCommandBuffers[current_frame], 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(mSwapchainExtent.width);
		viewport.height = static_cast<float>(mSwapchainExtent.height);
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


	bool RenderWindow::getSurfaceCapabilities(utility::ErrorState& errorState)
	{
		// Query the basic capabilities of a surface, needed in order to create a swapchain
		if (!errorState.check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mRenderService->getPhysicalDevice(), mSurface, &mSurfaceCapabilities) == VK_SUCCESS,
			"Unable to acquire surface capabilities"))
			return false;

		// Based on surface capabilities, determine swap image size
		if (mSurfaceCapabilities.currentExtent.width == UINT32_MAX)
		{
			glm::ivec2 buffer_size = getBufferSize();
			mSwapchainExtent =
			{
				math::clamp<uint32>(static_cast<uint32>(buffer_size.x), mSurfaceCapabilities.minImageExtent.width,  mSurfaceCapabilities.maxImageExtent.width),
				math::clamp<uint32>(static_cast<uint32>(buffer_size.y), mSurfaceCapabilities.minImageExtent.height, mSurfaceCapabilities.maxImageExtent.height),
			};
		}
		else
		{
			mSwapchainExtent = mSurfaceCapabilities.currentExtent;
		}
		return true;
	}


	void RenderWindow::setAlwaysOnTop(bool onTop)
	{
		SDL::setWindowAlwaysOnTop(mSDLWindow, onTop);
	}


	float RenderWindow::getDisplayScale() const
	{
		float scale;
		SDL::getWindowDisplayScale(mSDLWindow, &scale);
		return scale;
	}


	float RenderWindow::getPixelDensity() const
	{
		float density;
		SDL::getWindowPixelDensity(mSDLWindow, &density);
		return density;
	}


	int RenderWindow::getDisplayIndex() const
	{
		return mSDLWindow != nullptr ?
			SDL::getDisplayIndex(mSDLWindow) : -1;
	}


	bool RenderWindow::isMinimized() const
	{
		return (SDL::getWindowFlags(mSDLWindow) & SDL_WINDOW_MINIMIZED) != 0;
	}


	bool RenderWindow::isHidden() const
	{
		return (SDL::getWindowFlags(mSDLWindow) & SDL_WINDOW_HIDDEN) != 0;
	}


	bool RenderWindow::isOccluded() const
	{
		return (SDL::getWindowFlags(mSDLWindow) & SDL_WINDOW_OCCLUDED) != 0;
	}
}

