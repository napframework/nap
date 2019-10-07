#pragma once

// Local Includes
#include "glwindow.h"
#include "vulkan_fwd.h"

// External Includes
#include <rtti/rtti.h>
#include <utility/dllexport.h>

namespace nap
{
	class RendererSettings
	{
	public:
		bool mDoubleBuffer = true;			///< Property: 'DoubleBuffer' Enables / Disabled double buffering
		bool mEnableMultiSampling = true;	///< Property: 'EnableMultiSampling' Enables / Disables multi sampling.
		int  mMultiSamples = 4;				///< Property: 'MultiSampleSamples' Number of samples per pixel when multi sampling is enabled
		bool mEnableHighDPIMode = true;		///< Property: 'HighDPIMode' If high DPI render mode is enabled, on by default
	};

	/**
	 * OpenGL render back-end. 
	 * Initializes and shuts down the OpenGL API and allows for the creation of new render windows.
	 * This class also creates and manages the primary window which is constructed on initialization
	 */
	class NAPAPI Renderer final
	{
		RTTI_ENABLE()
	public:
		/**
		 * Delete copy constructor
		 */
		Renderer(const Renderer& other) = delete;
		Renderer& operator=(const Renderer& other) = delete;

		/**
		 * Default constructor and destructor
		 */
		~Renderer() = default;
		Renderer() = default;

		/**
		 * Create a new render window.
		 * @return the new render window or nullptr if unsuccessful
		 * @param settings the window settings used to create the window
		 * @param windowID the ID of the window to create
		 * @param errorState contains the error when creation fails.
		 */
		std::shared_ptr<GLWindow> createRenderWindow(const RenderWindowSettings& settings, const std::string& windowID, utility::ErrorState& errorState);

		/**
		 * Initialize the renderer.
		 * This call sets up the render attributes, create the first window and
		 * initializes the opengl engine. After initialization the primary window is active.
		 * @param rendererSettings settings used to initialize the renderer
		 * @param errorState contains the error when the renderer can't be initialized
		 * @return if the renderer initialized successfully
		 */
		bool init(const RendererSettings& rendererSettings, utility::ErrorState& errorState);

		/**
		 * Called when the renderer needs to shut down
		 */
		void shutdown();

		VkPhysicalDevice getPhysicalDevice() { return mPhysicalDevice; }
		VkDevice getDevice() { return mDevice; }
		VkCommandPool getCommandPool() { return mCommandPool; }

	private:
		RendererSettings				mSettings;				///< If high dpi render mode is enabled
		VkInstance						mInstance = nullptr;
		VkDebugReportCallbackEXT		mDebugCallback = nullptr;
		VkPhysicalDevice				mPhysicalDevice = nullptr;
		VkDevice						mDevice = nullptr;
		VkCommandPool					mCommandPool = nullptr;
		unsigned int					mGraphicsQueueIndex = -1;
	};
}
