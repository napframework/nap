/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "renderutils.h"
#include "irendertarget.h"

// External Includes
#include <window.h>
#include <rect.h>

// SDL Forward declares
struct SDL_Window;

namespace nap
{
	// Forward Declares
	class Core;
	class RenderService;

	/**
	 * Vulkan render window that can be declared in JSON. 
	 *
	 * Multiple presentation (display) modes are available to choose from. The selected presentation mode
	 * controls how the rendered images are presented to screen: in sync, out of sync or tied to the refresh rate of the monitor.
	 * It is however possible that the selected presentation mode is not supported by the graphics hardware,
	 * if that's the case the window will revert to FIFO_KRH, which is the presentation mode
	 * every vulkan compatible device must support. A warning is issued if the selected presentation mode
	 * is unavailable. 
	 *
	 * In order to improve performance, consider lowering the sample count and disable sample based shading.
	 * If sample based shading is requested but not supported, it is disabled and a warning is issued. 
	 * If the requested multi-sample count exceeds what is supported by the hardware, the highest available sample
	 * count is picked and a warning is issued.
	 *
	 * Initialization will fail when the nap::RenderService is configured to run headless.
	 * When headless rendering is enabled, the engine is initialized without surface and swapchain support,
	 * which are required by a nap::RenderWindow to display images on screen.
	 */
	class NAPAPI RenderWindow : public Window, public IRenderTarget
	{
		RTTI_ENABLE(Window)
		friend class RenderService;
	public:
		/**
		 * The various image presentation modes.
		 * Controls the way in which images are presented to screen. 
		 */
		enum class EPresentationMode : int
		{
			Immediate,					///< The new image immediately replaces the display image, screen tearing may occur. 
			Mailbox,					///< The new image replaces the one image waiting in the queue, becoming the first to be displayed. No screen tearing occurs. Could result in not-shown images. CPU not synced to display refresh rate.
			FIFO_Relaxed,				///< Every image is presented in order. No screen tearing occurs when drawing faster than monitor refresh rate. CPU synced to display refresh rate.
			FIFO						///< Every image is presented in order. No screen tearing occurs, also when drawing slower then monitor refresh rate. CPU synced to display refresh rate.
		};

		/**
		 * This constructor is called when creating the render window using the resource manager
		 * Every render window needs to be aware of it's render service
		 */
		RenderWindow(Core& core);

		/**
		 * Destroys all render resources
		 */
		virtual ~RenderWindow() override;

		/**
		 * Creates window, connects to resize event.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Called when the window is detroyed.
		 */
		virtual void onDestroy() override;

		/**
		 * Returns the width and height of this window, in screen coordinates.
		 * Note that on high DPI monitors this is not the same as the pixel count.
		 * To get the width and height in pixels use getBufferSize().
		 * @return the size of this window in pixels, ie: the drawable / buffer size
		 */
		const glm::ivec2 getSize() const;

		/**
         * Returns the width of the window, in screen coordinates.
         * Note that on high DPI monitors this is not the same as the pixel count.
         * To get the width in pixels use the size of the backbuffer using getWidthPixels().
		 * @return the width of the window
		 */
		int getWidth() const													{ return getSize().x; }

        /**
         * Returns the width of this window in pixels.
         * @return the width of the window in pixels.
         */
		int getWidthPixels() const												{ return getBufferSize().x; }
        
		/**
         * Returns the height of the window, in screen coordinates.
         * Note that on high DPI monitors this is not the same as the pixel count.
         * To get the height in pixels use the size of the backbuffer using getHeightPixels().
		 * @return the height of the window in pixels
		 */
		int getHeight() const													{ return getSize().y; }
        
        /**
         * Returns the height of this window in pixels.
         * @return the width of the window in pixels.
         */
		int getHeightPixels() const												{ return getBufferSize().y; }
        
		/**
		 * Returns the width and height of this window in pixels.
		 * @return the width and height of this window in pixels.
		 */
		const glm::ivec2 getBufferSize() const override;

		/**
		 * Shows the window and gives it input focus.
		 * This call also makes sure the window is on top of other windows.
		 */
		void show();

		/**
		 *	Hides the window
		 */
		void hide();

		/**
		 *	@return the window title
		 */
		const std::string& getTitle() const										{ return mTitle; }

		/**
		 *	Sets the window title
		 */
		void setTitle(std::string newTitle);

		/**
		 *	@return if the window is resizable
		 */
		bool isResizable() const												{ return mResizable; }
	
		/**
		 * Turns full screen on / off
		 * This is the windowed full screen mode, game is not supported
		 * @param value if the window is set to fill the screen or not
		 */
		void setFullscreen(bool value);

		/**
		 * Toggles full screen on / off
		 */
		void toggleFullscreen();

		/**
		 * Sets the width of the window, in screen coordinates.
		 * @param width the new width of the window, in screen coordinates.
		 */
		void setWidth(int width);

		/**
         * Sets the height of the window, in screen coordinates.
		 * @param height the new window height, in screen coordinates.
		 */
		void setHeight(int height);

		/**
		 * Set the size of the window, in screen coordinates.
		 * @param size the new window size, in screen coordinates.
		 */
		void setSize(const glm::ivec2& size);

		/**
		 * Sets the window clear color.
		 * @param color the new clear color
		 */
		virtual void setClearColor(const RGBAColorFloat& color) override;

		/**
		 * Returns the window clear color.
		 * @return the window clear color.
		 */
		virtual const RGBAColorFloat& getClearColor() const override;

		/**
		 * Sets the position of the window on screen
		 * @param position the new screen position in pixel coordinates
		 */
		void setPosition(const glm::ivec2& position);

		/**
		 * @return the window position in pixel coordinates
		 */
		const glm::ivec2 getPosition() const;

		/**
		 * @return the hardware window handle, nullptr if undefined
		 */
		SDL_Window* getNativeWindow() const;

		/**
		 *	@return the hardware window number
		 */
		virtual uint getNumber() const override;

		/**
		 * Creates a rectangle based on the current width and height of the render window.
		 * Note that the returned dimensions of the rectangle can differ from the actual size in pixels on a high dpi monitor.
		 * To obtain a rectangle that contains the actual size of the window in pixels use: getRectPixels
		 * @return the window as a rectangle
		 */
		math::Rect getRect() const;

		/**
		 * Creates a rectangle based on the current width and height of the render window in pixels.
		 * @return the window as rectangle
		 */
		math::Rect getRectPixels() const;

		/**
		 * Starts a render pass. Only call this when recording is enabled.
		 */
		virtual void beginRendering() override;

		/**
		 * Ends a render pass. Always call this after beginRendering().
		 */
		virtual void endRendering() override;

		/**
		 * @return swapchain format used to render to window.
		 */
		virtual VkFormat getColorFormat() const override							{ return mSwapchainFormat; }

		/**
		 * @return depth format used by window.
		 */
		virtual VkFormat getDepthFormat() const override;
		
		/**
		 * @return used number of samples when rendering to the window.
		 */
		virtual VkSampleCountFlagBits getSampleCount() const override				{ return mRasterizationSamples; }
		
		/**
		 * @return if sample based shading is enabled when rendering to the window.
		 */
		virtual bool getSampleShadingEnabled() const override						{ return mSampleShadingEnabled; }

		/**
		 * @return polygon winding order
		 */
		virtual ECullWindingOrder getWindingOrder() const override;

		/**
		 * @return render pass associated with this window.
		 */
		virtual VkRenderPass getRenderPass() const override							{ return mRenderPass; }

		bool					mSampleShading		= true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		int						mWidth				= 512;								///< Property: 'Width' window horizontal resolution
		int						mHeight				= 512;								///< Property: 'Height' window vertical resolution
		bool					mBorderless			= false;							///< Property: 'Borderless' if the window has any borders
		bool					mResizable			= true;								///< Property: 'Resizable' if the window is resizable
		bool					mVisible			= true;								///< Property: 'Visible' if the render window is visible on screen
		EPresentationMode		mMode				= EPresentationMode::Immediate;		///< Property: 'Mode' the image presentation mode to use
		std::string				mTitle				= "";								///< Property: 'Title' window title
		RGBAColorFloat			mClearColor			= { 0.0f, 0.0f, 0.0f, 1.0f };		///< Property: 'ClearColor' background clear color
		ERasterizationSamples	mRequestedSamples	= ERasterizationSamples::Four;		///< Property: 'Samples' The number of samples used during Rasterization. For even better results enable 'SampleShading'.
		int						mAddedSwapImages	= 1;								///< Property: 'AdditionalSwapImages' number of additional swapchain images to create, added to minimum specified by hardware.
		bool					mRestorePosition	= true;								///< Property: 'RestorePosition' if window position is restored from previous session
		bool					mRestoreSize		= true;								///< Property: 'RestoreSize' if window size is restored from previous session	

	private:
		RenderService*					mRenderService	= nullptr;
		SDL_Window*						mSDLWindow		= nullptr;						
		VkSampleCountFlagBits			mRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkDevice						mDevice = VK_NULL_HANDLE;
		VkSurfaceKHR					mSurface = VK_NULL_HANDLE;
		VkSwapchainKHR					mSwapchain = VK_NULL_HANDLE;
		VkRenderPass					mRenderPass = VK_NULL_HANDLE;
		VkQueue							mPresentQueue = VK_NULL_HANDLE;
		VkFormat						mSwapchainFormat = VK_FORMAT_UNDEFINED;
		std::vector<VkImageView>		mSwapChainImageViews;
		std::vector<VkFramebuffer>		mSwapChainFramebuffers;
		std::vector<VkCommandBuffer>	mCommandBuffers;
		std::vector<VkSemaphore>		mImageAvailableSemaphores;
		std::vector<VkSemaphore>		mRenderFinishedSemaphores;
		std::vector<int>				mImagesInFlight;
		VkPresentModeKHR				mPresentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
		ImageData						mDepthImage;
		ImageData						mColorImage;
		bool							mSampleShadingEnabled = false;
		uint32							mCurrentImageIndex = 0;
		uint32							mSwapChainImageCount = 0;
		bool							mRecreateSwapchain = false;
		VkSurfaceCapabilitiesKHR		mSurfaceCapabilities;
		VkExtent2D						mSwapchainExtent = {0,0};
		bool							mToggleFullscreen = false;

		/**
		 * Called by the render service. 
		 * Starts a new record operation for this window, returns a 
		 * VK_NULL_HANDLE if there is nothing to record.
		 * @return the command buffer currently recorded for this frame, VK_NULL_HANDLE if nothing to record.
		 */
		VkCommandBuffer beginRecording();

		/**
		 * Called by the render service.
		 * Ends the recording operation, submits the queue and asks for presentation.
		 */
		void endRecording();
		
		/**
		 * Checks if the event is a window resize event and updates size accordingly.
		 */
		void handleEvent(const Event& event);

		/**
		 * Obtain the surface properties that are required for the creation of the swap chain 
		 */
		bool getSurfaceCapabilities(utility::ErrorState& error);

		/**
		 * Destroys currently active swapchain and creates a new one based on the current window size and settings.
		 * @param errorState contains the error if creation fails
		 * @return if the swapchain has been recreated.
		 */
		bool recreateSwapChain(utility::ErrorState& errorState);
		
		/**
		 * Creates the swapchain based on the current window size and settings
		 * @param errorState contains the error if creation fails.
		 * @return if creation succeeded
		 */
		bool createSwapChainResources(utility::ErrorState& errorState);

		/**
		 * Destroys all Vulkan swapchain related resources.
		 */
		void destroySwapChainResources();

		/**
		 * @return if the swapchain extent is higher than zero in both axis
		 */
		bool validSwapchainExtent() const;
	};
}
