/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "vk_mem_alloc.h"
#include "pipelinekey.h"
#include "renderutils.h"

// External Includes
#include <nap/service.h>
#include <windowevent.h>
#include <rendertarget.h>
#include <material.h>

namespace nap
{
	// Forward Declares
	class CameraComponentInstance;
	class RenderableComponentInstance;
	class RenderWindow;
	class RenderService;
	class SceneService;
	class DescriptorSetCache;
	class DescriptorSetAllocator;
	class RenderableMesh;
	class IMesh;
	class MaterialInstance;
	class Texture2D;
	class GPUBuffer;

	//////////////////////////////////////////////////////////////////////////
	// Render Service Configuration
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Render engine configuration settings.
	 */
	class NAPAPI RenderServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:
		/**
		 * Supported Vulkan device types. 
		 */
		enum class EPhysicalDeviceType : int
		{
			Integrated	= 1,	///< Integrated graphics card
			Discrete	= 2,	///< Discrete (dedicated) graphics card
			Virtual		= 3,	///< Virtual graphics card
			CPU			= 4		///< CPU as graphics card
		};

		bool						mHeadless = false;												///< Property: 'Headless' Render without a window. Turning this on forbids the use of a nap::RenderWindow.
		EPhysicalDeviceType			mPreferredGPU = EPhysicalDeviceType::Discrete;					///< Property: 'PreferredGPU' The preferred type of GPU to use. When unavailable, the first GPU in the list is selected. 
		bool						mEnableHighDPIMode = true;										///< Property: 'EnableHighDPI' If high DPI render mode is enabled, on by default
		uint32						mVulkanVersionMajor = 1;										///< Property: 'VulkanMajor The major required vulkan API instance version.
		uint32						mVulkanVersionMinor = 0;										///< Property: 'VulkanMinor' The minor required vulkan API instance version.
		std::vector<std::string>	mLayers = { "VK_LAYER_KHRONOS_validation" };			        ///< Property: 'Layers' Vulkan layers the engine tries to load in Debug mode. Warning is issued if the layer can't be loaded. Layers are disabled in release mode.
		std::vector<std::string>	mAdditionalExtensions = { };									///< Property: 'Extensions' Additional required Vulkan device extensions
		bool						mPrintAvailableLayers = false;									///< Property: 'ShowLayers' If all the available Vulkan layers are printed to console
		bool						mPrintAvailableExtensions = false;								///< Property: 'ShowExtensions' If all the available Vulkan extensions are printed to console
		uint32						mAnisotropicFilterSamples = 8;									///< Property: 'AnisotropicSamples' Default max number of anisotropic filter samples, can be overridden by a sampler if required.
		virtual rtti::TypeInfo		getServiceType() const override									{ return RTTI_OF(RenderService); }
	};


	//////////////////////////////////////////////////////////////////////////
	// Render Physical Device
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Vulkan physical device (GPU), binds together physical device information for better management.
	 */
	class NAPAPI PhysicalDevice
	{
	public:
		// Default constructor, invalid object
		PhysicalDevice() = default;

		// Called by the render service
		PhysicalDevice(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties, int queueIndex);

		/**
		 * @return Physical device handle
		 */
		VkPhysicalDevice getHandle() const { return mDevice; }

		/**
		 * @return queue index used for graphics commands and image presentation.
		 */
		int getQueueIndex() const { return mQueueIndex; }

		/**
		 * @return physical device properties
		 */
		const VkPhysicalDeviceProperties& getProperties() const { return mProperties; }

		/**
		 * @return Physical device features
		 */
		const VkPhysicalDeviceFeatures& getFeatures() const { return mFeatures; }

		/**
		 * @return if the device is valid
		 */
		bool isValid() const	{ return mDevice != VK_NULL_HANDLE && mQueueIndex >= 0; }

	private:
		VkPhysicalDevice			mDevice = VK_NULL_HANDLE;			///< Handle to physical device
		VkPhysicalDeviceProperties	mProperties;						///< Properties of the physical device
		VkPhysicalDeviceFeatures	mFeatures;							///< Physical device features
		int							mQueueIndex = -1;					///< Graphics queue index
	};


	//////////////////////////////////////////////////////////////////////////
	// Render Service
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Main interface for 2D and 3D rendering operations.
	 *
	 * This service initializes the Vulkan back-end and provides an interface to render objects to a specific target (screen or back-buffer).
	 * The service is shut down automatically on exit, and destroys all left over resources.
	 * When rendering geometry the service automatically sorts your selection based on the blend mode of the material.
	 * Opaque objects are rendered front to back, alpha blended objects are rendered back to front.
	 *
	 * By default headless rendering is turned off. This means Vulkan is setup to display the result of a render operation 
	 * in a nap::RenderWindow, next to a nap::RenderTarget. This requires a display device to be connected to the system.
	 * Enable headless rendering when you do NOT want to render to a window or when there is no display attached to the system.
	 * This in turn forbids the use of a nap::RenderWindow inside your application.
	 *
	 * When headless rendering is enabled, the engine will be initialized  without surface and swapchain support, 
	 * which are required by a nap::RenderWindow to display images on screen. You can therefore only render to a 
	 * nap::RenderTarget when Headless rendering is enabled. 
	 *
	 * Turn headless rendering on / off using the nap::RenderServiceConfiguration.
	 *
	 * The service creates a Vulkan 1.0 instance by default, but applications may use Vulkan 1.1 and 1.2 functionality if required.
	 * Make sure to set the required major and minor Vulkan version accordingly using the RenderServiceConfiguration.
	 * The application will not start if the device does not support the selected (and therefore required) version of Vulkan.
	 *
	 * The following Vulkan device extensions are always required: VK_KHR_MAINTENANCE1_EXTENSION.
	 * When rendering to a window, the VK_KHR_SWAPCHAIN_EXTENSION is also required.
	 * Additional extension can be specified using the nap::RenderServiceConfiguration.
	 *
	 * The system will try to load the requested validation layers in debug mode only.
	 * Use the RenderServiceConfiguration to specify which layers the Vulkan loader should attempt to load.
	 * A warning is issued when the validation layer can't be located or loaded. Validation layers are disabled in release mode.
	 *
	 * For more information on setting up validation layers refer to:
	 * https://vulkan.lunarg.com/doc/view/1.2.131.2/windows/layer_configuration.html
	 *
	 * On initialization the service will try to choose a physical device based on the preferred GPU type.
	 * If no compatible GPU is found (even a not-preferred one) the system will fail to initialize.
	 * Most dedicated and integrated GPUs are supported.
	*/
	class NAPAPI RenderService : public Service
	{
		friend class Texture2D;
		friend class GPUBuffer;
		RTTI_ENABLE(Service)
	public:
		using SortFunction = std::function<void(std::vector<RenderableComponentInstance*>&, const CameraComponentInstance&)>;
		using VulkanObjectDestructor = std::function<void(RenderService&)>;
		
		/**
		 * Binds a pipeline and pipeline layout together.
		 */
		struct Pipeline
		{
			/**
			 * Returns if the pipeline has been created and is set.
			 * @return if the pipeline has been created and is set.
			 */
			bool isValid() const	{ return mPipeline != nullptr && mLayout != nullptr; }

			VkPipeline				mPipeline = nullptr;	///< Handle to Vulkan pipeline
			VkPipelineLayout		mLayout = nullptr;		///< Handle to Vulkan pipeline layout
		};

		/**
		 * Creates the service together with the provided configuration settings.
		 * @param configuration render engine configuration.
		 */
		RenderService(ServiceConfiguration* configuration);

		// Default destructor
		virtual ~RenderService();

		/**
		 * Tells the system you are about to render a new frame.
		 * The system might wait until all commands issued previously, associated with the same frame handle (fence), have been processed.
		 * Multiple frames are in flight at the same time, but if the graphics load is heavy, the system might wait here to ensure resources are available.
		 * Pending upload requests are executed. Previously issued download requests are checked for completion. 
		 * Queued resources that are out of scope are destroyed.
		 *
		 * Call this function at the beginning of the render loop, before any RenderService::beginRecording() calls.
		 * Make sure to call RenderService::endFrame() after all recording operations are finished, at the end of the render loop.
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginRecording(*mRenderWindow))
		 *		{
		 *			...
		 *			mRenderService->endRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 */
		void beginFrame();
		
		/**
		 * Tells the system you finished rendering into the frame.
		 * Always call this at the end of the render() loop, after RenderService::beginFrame().
		 * Failure to properly end the frame will result in a system freeze.
		 * Any pending download requests are pushed onto the command buffer.
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginRecording(*mRenderWindow))
		 *		{
		 *			...
		 *			mRenderService->endRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 */
		void endFrame();

		/**
		 * Starts a headless render operation.
		 * Call this when you want to render objects to a render-target instead of a render window.
		 * Make sure to call RenderService::endHeadlessRecording() afterwards.
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginHeadlessRecording())
		 *		{
		 *			...
		 *			mRenderService->endHeadlessRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 * @return if the headless record operation started successfully.
		 */
		bool beginHeadlessRecording();
		
		/**
		 * Ends a headless render operation, submits the recorded command buffer to the queue.
		 * Always call this function after a successful call to nap::RenderService::beginHeadlessRecording().
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginHeadlessRecording())
		 *		{
		 *			...
		 *			mRenderService->endHeadlessRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 */
		void endHeadlessRecording();

		/**
		 * Starts a window render operation. Call this when you want to render geometry to a render window.
		 * Always call RenderService::endRecording() afterwards, on success.
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginRecording(*mRenderWindow))
		 *		{
		 *			...
		 *			mRenderService->endRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 * @return if the window record operation started successfully.
		 */
		bool beginRecording(RenderWindow& renderWindow);
		
		/**
		 * Ends a window render operation, submits the recorded command buffer to the queue.
		 * Always call this function after a successful call to nap::RenderService::beginRecording().
		 *
		 * ~~~~~{.cpp}
		 *		mRenderService->beginFrame();
		 *		if (mRenderService->beginRecording(*mRenderWindow))
		 *		{
		 *			...
		 *			mRenderService->endRecording();
		 *		}
		 *		mRenderService->endFrame();
		 * ~~~~~
		 */
		void endRecording();

		/**
		 * Renders all available nap::RenderableComponent(s) in the scene to a specific renderTarget.
		 * The objects to render are sorted using the default sort function (front-to-back for opaque objects, back-to-front for transparent objects).
		 * The sort function is provided by the render service itself, using the default NAP DepthSorter.
		 * Components that can't be rendered with the given camera are omitted.
		 * @param renderTarget the target to render to
		 * @param camera the camera used for rendering all the available components
		 */
		void renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera);

		/**
		 * Renders all available nap::RenderableComponent(s) in the scene to a specific renderTarget.
		 * Components that can't be rendered with the given camera are omitted.
		 * @param renderTarget the target to render to
		 * @param camera the camera used for rendering all the available components
		 * @param sortFunction The function used to sort the components to render
		 */
		void renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera, const SortFunction& sortFunction);

		/**
		 * Renders a specific set of objects to a specific renderTarget.
		 * The objects to render are sorted using the default sort function (front-to-back for opaque objects, back-to-front for transparent objects)
		 * The sort function is provided by the render service itself, using the default NAP DepthSorter.
		 * @param renderTarget the target to render to
		 * @param camera the camera used for rendering all the available components
		 * @param comps the components to render to renderTarget
		 */
		void renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps);

		/**
		* Renders a specific set of objects to a specific renderTarget.
		*
		* @param renderTarget the target to render to
		* @param camera the camera used for rendering all the available components
		* @param comps the components to render to renderTarget
		* @param sortFunction The function used to sort the components to render
		*/
		void renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps, const SortFunction& sortFunction);

		/**
		 * Add a new window as target to the render engine.
		 * @param window the window to add as a valid render target
		 * @param errorState contains the error message if the window could not be added
		 */
		bool addWindow(RenderWindow& window, utility::ErrorState& errorState);

		/**
		 * Remove a window as a valid target from the render engine.
		 * @param window the window to remove from the render service
		 */
		void removeWindow(RenderWindow& window);

		/**
		 * Find a RenderWindowResource by its native handle
		 * @param nativeWindow the native window handle (i.e. the SDL_Window pointer)
		 * @return the render window associated with the native window
		 */
		RenderWindow* findWindow(void* nativeWindow) const;

		/**
		 * Find a RenderWindow based on a window number.
		 * @param id the number of the window to find.
		 * @return the window, nullptr if not found
		 */
		RenderWindow* findWindow(uint id) const;

		/**
		 * Add a window event that is processed later, ownership is transferred here.
		 * The window number in the event is used to find the right render window to forward the event to.
		 * @param windowEvent the window event to add.
		 */
		void addEvent(WindowEventPtr windowEvent);

		/**
		 * Creates a renderable mesh, that represents the coupling between a mesh and material, that can be rendered to screen.
		 * Internally the renderable mesh manages a vertex array object that is issued by the render service.
		 * This function should be called from on initialization of components that work with meshes and materials: ie: all types of RenderableComponent. 
		 * The result should be validated by calling RenderableMesh.isValid(). Invalid mesh / material representations can't be rendered together.
		 * @param mesh The mesh that is used in the mesh-material combination.
		 * @param materialInstance The material instance that is used in the mesh-material combination.
		 * @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		 * @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		 */
		RenderableMesh createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState);

		/**
		 * Returns a Vulkan pipeline for the given render target, mesh and material combination.
		 * Internally pipelines are cached, a new pipeline is created when a new combination is encountered.
		 * Because of this initial frames are slower to render, until all combinations are cached and returned from the pool.
		 * Pipeline creation is considered to be a heavy operation, take this into account when designing your application.
		 *
		 * Use this function inside nap::RenderableComponentInstance::onDraw() to find the right pipeline before rendering.
		 * ~~~~~{.cpp}
		 *		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(target, mesh, mat_instance, error_state);
		 *		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		 * ~~~~~
		 *
		 * @param renderTarget target that is rendered too.
		 * @param mesh the mesh that is drawn.
		 * @param materialInstance the material applied to the mesh.
		 * @param errorState contains the error if the pipeline can't be created
		 * @return new or cached pipeline.
		 */
		Pipeline getOrCreatePipeline(const IRenderTarget& renderTarget, const IMesh& mesh, const MaterialInstance& materialInstance, utility::ErrorState& errorState);

		/**
		 * Returns a Vulkan pipeline for the given render target and Renderable-mesh combination.
		 * Internally pipelines are cached, a new pipeline is created when a new combination is encountered.
		 * Because of this initial frames are slower to render, until all combinations are cached and returned from the pool.
		 * Pipeline creation is considered to be a heavy operation, take this into account when designing your application.
		 *
		 * Use this function inside nap::RenderableComponentInstance::onDraw() to find the right pipeline before rendering.
		 * ~~~~~{.cpp}
		 *		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(target, render_mesh, error_state);
		 *		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		 * ~~~~~
		 *
		 * @param renderTarget target that is rendered too.
		 * @param renderableMesh the mesh / material combination that is rendered
		 * @param errorState contains the error if the pipeline can't be created
		 * @return new or cached pipeline.
		 */
		Pipeline getOrCreatePipeline(const IRenderTarget& renderTarget, const RenderableMesh& renderableMesh, utility::ErrorState& errorState);

		/**
		 * Queues a function that destroys Vulkan resources when appropriate.
		 * Certain Vulkan resources, including buffers, image buffers etc. might still be in use when
		 * the NAP resource is destroyed. It is therefore necessary to queue their destruction, instead
		 * of deleting them immediately. Make sure that all resources are captured by copy, instead of reference.
		 *
		 * ~~~~~{.cpp}
		 * 		mRenderService->queueVulkanObjectDestructor([buffers = mRenderBuffers](RenderService& service)
		 *		{
		 *			for (const BufferData& buffer : buffers)
		 *			{
		 *				destroyBuffer(service.getVulkanAllocator(), buffer);
		 *			}
		 *		});
		 * ~~~~~
		 */
		void queueVulkanObjectDestructor(const VulkanObjectDestructor& function);

		/**
		 * Returns a descriptor set cache based on the given layout.
		 * The cache is used to acquire descriptor sets.
		 * @param layout the descriptor set layout to create the cache for.
		 * @return descriptor set cache for the given layout.
		 */
		DescriptorSetCache& getOrCreateDescriptorSetCache(VkDescriptorSetLayout layout);

		/**
		 * @return main Vulkan allocator
		 */
		VmaAllocator getVulkanAllocator() const { return mVulkanAllocator; }
		
		/**
		 * Returns if the render engine runs headless. 
		 * This allows you to render images without any display device.
		 * This in turn means that when enabled it is not possible to display (present) images to a window.
		 * @return if the render engine runs headless.
		 */
		bool isHeadless() const														{ return mHeadless; }

		/**
		 * Returns the command buffer that is being recorded. Every window records into
		 * it's own command buffer. All headless render operations share the same command buffer.
		 * The current command buffer is set after nap::beginHeadlessRecording() or
		 * nap::beginRecording(RenderWindow&) and only valid until the recording operation is ended.
		 * @return the command buffer that is being recorded.
		 */
		VkCommandBuffer getCurrentCommandBuffer()									{ assert(mCurrentCommandBuffer != nullptr); return mCurrentCommandBuffer; }
		
		/**
		 * Returns the window that is being rendered to, only valid between a
		 * successfull call to: RenderService::beginRecording() and RenderService::endRecording().
		 * @return the window currently being rendered to, nullptr if not set.
		 */
		RenderWindow* getCurrentRenderWindow()										{ assert(mCurrentRenderWindow != nullptr); return mCurrentRenderWindow; }

		/**
		 * Returns the Vulkan runtime instance.
		 * @return Vulkan runtime instance.
		 */
		VkInstance getVulkanInstance() const										{ return mInstance; }

		/**
		 * Returns the physical device (GPU) that is used for all render operations.
		 * Multiple physical devices are not supported.
		 * @return Selected Vulkan hardware (GPU) device
		 */
		VkPhysicalDevice getPhysicalDevice() const									{ return mPhysicalDevice.getHandle(); }

		/**
		 * Returns all supported physical device (GPU) features.
		 * @return all supported hardware features
		 */
		const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const			{ return mPhysicalDevice.getFeatures(); }

		/**
		 * Returns the Vulkan api version, as supported by the physical device.
		 * Note that this is not the same as the api version used by the render engine.
		 * The physical device needs to support the min required api version requested by the render engine,
		 * but could be higher.
		 * @return the version of Vulkan supported by the device
		 */
		uint32_t getPhysicalDeviceVersion() const									{ return mPhysicalDevice.getProperties().apiVersion; }

		/**
		 * All physical device hardware properties.
		 * @return all hardware properties
		 */
		const VkPhysicalDeviceProperties&	getPhysicalDeviceProperties() const		{ return mPhysicalDevice.getProperties(); }

		/**
		 * Returns the handle to the logical Vulkan device,
		 * represents the hardware together with the extensions, selected queue and features enabled for it.
		 * @return The logical Vulkan device.
		 */
		VkDevice getDevice() const													{ return mDevice; }

		/**
		 * Returns the max number of hardware supported rasterization samples.
		 * @return the max number of rasterization samples supported by the hardware.
		 */
		VkSampleCountFlagBits getMaxRasterizationSamples() const;

		/**
		 * Returns max supported rasterization samples based on the requested number of samples.
		 * The output is automatically clamped if the requested number of samples exceeds the hardware limit.
		 * @return if requested number of samples is supported by hardware.
		 * @param requestedSamples requested number of samples.
		 * @param outSamples supported number of samples.
		 * @param errorState contains the error if requested number of samples is not supported by the hardware.
		 */
		bool getRasterizationSamples(ERasterizationSamples requestedSamples, VkSampleCountFlagBits& outSamples, nap::utility::ErrorState& errorState);

		/**
		 * Returns if sample shading is supported and enabled, reduces texture aliasing at computational cost.
		 * @return if sample shading is supported.
		 */
		bool sampleShadingSupported() const;

		/**
		 * Returns if anisotropic filtering is supported
		 * @return if anisotropic filtering is supported
		 */
		bool anisotropicFilteringSupported() const;

		/**
		 * Returns if wide line rendering is supported, if so you can use a line width higher than 1.
		 * @return if wide line rendering is supported.
		 */
		bool getWideLinesSupported() const											{ return mWideLinesSupported; }

		/**
		 * Returns if rendering large points is supported.
		 * @return if rendering large points is supported.
		 */
		bool getLargePointsSupported() const										{ return mLargePointsSupported; }

		/**
		 * Returns the (system default) number of anisotropic filter samples. 
		 * The output is always 1 when anisotropic filtering is not supported.
		 * @return system default number of max anisotropic filter samples.
		 */
		uint32 getAnisotropicSamples() const										{ return mAnisotropicSamples; }

		/**
		 * Returns the selected and currently in use depth format.
		 * @return the currently selected and in use depth format.
		 */
		VkFormat getDepthFormat() const												{ return mDepthFormat; }

		/**
		 * Returns a handle to the Vulkan command pool object.
		 * Command pools are opaque objects that command buffer memory is allocated from.
		 * @return handle to the Vulkan command pool object.
		 */
		VkCommandPool getCommandPool() const										{ return mCommandPool; }
		
		/**
		 * @return flags that specify which depth aspects of an image are included in a view.
		 */
		VkImageAspectFlags getDepthAspectFlags() const;
		
		/**
		 * Returns the index of the selected queue family.
		 * @return the main queue index.
		 */
		uint32 getQueueIndex() const												{ return mPhysicalDevice.getQueueIndex(); }
		
		/**
		 * Returns the selected queue, used to execute recorded command buffers.
		 * The queue must support Graphics and Transfer operations.
		 * @return the queue that is used to execute recorded command buffers.
		 */
		VkQueue getQueue() const													{ return mQueue; }

		/**
		 * Returns an empty texture that is available on the GPU for temporary biding or storage.
		 * @return empty texture that is available on the GPU
		 */
		Texture2D& getEmptyTexture() const											{ return *mEmptyTexture; }

		/**
		 * Returns an existing or new material for the given type of shader that can be shared.
		 * This only works for hard coded shader types that can be initialized without input arguments.
		 * If initialization or creation fails, the result is cached but invalid.
		 * Use the material as a template for a material instance.
		 *
		 * ~~~~~{.cpp}
		 *	mRenderService->getOrCreateMaterial(RTTI_OF(nap::FontShader), error);
		 * ~~~~~
		 *
		 * @param shaderType type of shader to get material for.
		 * @param error contains the error if the material could not be created or initialized.
		 * @return new or existing material for the given shader type, nullptr if creation or initialization failed.
		 */
		Material* getOrCreateMaterial(rtti::TypeInfo shaderType, utility::ErrorState& error);

		/**
	 	 * Returns an existing or new material for the given type of shader T that can be shared.
		 * This only works for hard coded shader types that can be initialized without input arguments.
		 * If initialization or creation fails, the result is cached but invalid.
		 * Use the material as a template for a material instance.
		 *
		 * ~~~~~{.cpp}
		 *	mRenderService->getOrCreateMaterial<nap::FontShader>(error);
		 * ~~~~~
		 *
		 * @param error contains the error if the material could not be created or initialized.
		 * @return new or existing material for the given shader type, nullptr if creation or initialization failed.
		 */
		template<typename T>
		Material* getOrCreateMaterial(utility::ErrorState& error)					{ return getOrCreateMaterial(RTTI_OF(T), error); }

		/**
		 * Returns the index of the frame that is currently rendered.
		 * This index controls which command buffer is recorded and is therefore
		 * capped to the max number of images in flight.
		 * @return index of the currently rendered frame.
		 */
		int getCurrentFrameIndex() const											{ return mCurrentFrameIndex; }

		/**
		 * Returns the max number of frames in flight. If there is only
		 * 1 frame in flight the application will stall until it is rendered. Having
		 * multiple frames in flight at once allows the render engine to start on a new frame
		 * without having to wait on the previous one to finish.
		 *
		 * Increasing the number of frames in flight does however have a negative impact on resource usage,
		 * because every frame requires its own unique set of command buffers, descriptor sets etc.
		 * 2 is therefore a good number, where 3 offers only, in most situations, a slight increase in performance.
		 * This however greatly depends on the application GPU and CPU load.
		 */
		int getMaxFramesInFlight() const											{ return 2; }

		/**
		 * Returns the physical device properties for the requested Vulkan format.
		 * @return physical device properties for the requested Vulkan format.
		 */
		void getFormatProperties(VkFormat format, VkFormatProperties& outProperties);

		/**
		 * Returns if the render service is currently recording (rendering) a frame.
		 * Returns true in between start and end frame calls, otherwise false.
		 * If that is the case, certain operations are not allowed.
		 * For example: data upload / download operations to and from the GPU are forbidden.
		 * @return if the render service is currently recording a frame.
		 */
		bool isRenderingFrame() const												{ return mIsRenderingFrame; }

		/**
		 * Returns the api version used to create the Vulkan instance.
		 * Note that the physical device is required to support at least that version.
		 * @return The api version used to create the Vulkan instance.
		 */
		uint32 getVulkanVersion() const												{ return mAPIVersion; }

		/**
		 * Returns the major api version used to create the Vulkan instance.
		 * The vulkan instance is created using a combination of the major and minor api version.
		 * Note that the physical device is required to support at least that version.
		 * @return Vulkan major api version
		 */
		uint32 getVulkanVersionMajor() const;

		/**
		 * Returns the minor api version used to create the Vulkan instance.
		 * The vulkan instance is created using a combination of the major and minor api version.
		 * Note that the physical device is required to support at least that version.
		 * @return Vulkan minor api version
		 */
		uint32 getVulkanVersionMinor() const;

		/**
		 * Called when a new window is added to the system
		 */
		nap::Signal<nap::RenderWindow&> windowAdded;

		/**
		 * Called just before a window is removed from the system
		 */
		nap::Signal<nap::RenderWindow&> windowRemoved;

		/**
		 * Returns if the render service is running, a non operational render service is
		 * in an undefined state and can therefore not be queried
		 * @return if the render service is initialized and therefore running
		 */
		bool isInitialized() const	{ return mInitialized; }

		/**
		 * Wait for the fence belonging to the specified frame index. This ensures that, after the wait, all resources for that frame are no longer in use.
		 * You normally don't need to use this as all synchronization is handled for you. This function is only used by RenderWindow to ensure that the current
		 * swap chain image is no longer in use.
		 *
		 * @param frameIndex The index of the frame to wait for
		 */
		void waitForFence(int frameIndex);

	protected:
		/**
		 * Register dependencies, render module depends on scene
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Initialize the renderer, the service owns the renderer.
		 * @param errorState contains the error message if the service could not be initialized
		 * @return if the service has been initialized successfully
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * Waits for the device to be idle and deletes queued resources.
		 */
		virtual void preShutdown() override;

		/**
		 * Shuts down the renderer, local Vulkan resources are destroyed.
		 */
		virtual void shutdown() override;

		/**
		 * Invoked when the resource manager is about to load resources.
		 */
		virtual void preResourcesLoaded() override;

		/**
		 * Process all received window events.
		 * @param deltaTime time in seconds in between frames.
		 */
		virtual void update(double deltaTime) override;

    private:
		/**
		 * Sorts a set of renderable components based on distance to the camera, ie: depth
		 * Note that when the object is of a type mesh it will use the material to sort based on opacity
		 * If the renderable object is not a mesh the sorting will occur front-to-back regardless of it's type as we don't
		 * know the way the object is rendered to screen
		 * @param comps the renderable components to sort
		 * @param camera the camera used for sorting based on distance
		 */
		void sortObjects(std::vector<RenderableComponentInstance*>& comps, const CameraComponentInstance& camera);

		/**
		 * Initializes the empty texture, fills it with zero. The texture is uploaded at the beginning of the next frame.
		 * @param errorState contains the error if the texture could not be initialized
		 * @return if the texture initialized successfully.
		 */
		bool initEmptyTexture(nap::utility::ErrorState& errorState);

		/**
		 * Deletes all texture upload and download requests.
		 * Called when a texture resource is destroyed.
		 * @param texture the texture to remove upload and download requests for.
		 */
		void removeTextureRequests(Texture2D& texture);

		/**
		* Request a texture clear
		* @param texture the texture to clear.
		*/
		void requestTextureClear(Texture2D& texture);

		/**
		 * Request a pixel data transfer, from a staging buffer to image buffer.
		 * @param texture the texture to upload.
		 */
		void requestTextureUpload(Texture2D& texture);

		/**
		 * Request a pixel data transfer, from image buffer to CPU.
		 * @param texture the texture to download data into.
		 */
		void requestTextureDownload(Texture2D& texture);

		/**
		 * Request a Vulkan buffer transfer, from staging buffer to GPU.
		 * @param buffer the butter to upload to the GPU.
		 */
		void requestBufferUpload(GPUBuffer& buffer);

		/**
		 * Deletes all buffer upload requests.
		 * Called when the GPU buffer is destroyed.
		 * @param buffer the buffer to remove pending upload commands for.
		 */
		void removeBufferRequests(GPUBuffer& buffer);

		/**
		 * Transfers all previously queued data to the GPU.
		 * @param commandBuffer the command buffer to record the transfer operations in.
		 * @param transferFunction function that is called at the appropriate time to upload the data.
		 */
		void transferData(VkCommandBuffer commandBuffer, const std::function<void()>& transferFunction);
		
		/**
		 * Downloads all previously queued data from the GPU.
		 */
		void downloadData();
		
		/**
		 * Called by transferdata(), uploads all texture and buffer data to the GPU.
		 */
		void uploadData();

		/**
		 * Checks if any pending texture downloads are ready.
		 * Textures for which the download has completed are notified.
		 */
		void updateTextureDownloads();
		
		/**
		 * Called by the render engine at the appropriate time to delete all queued Vulkan resources.
		 * @param frameIndex index of the frame to delete resources for.
		 */
		void processVulkanDestructors(int frameIndex);

		/**
		 * Waits for the device to reach idle state, when reached all queued destructions are processed.
		 * A device in idle state is forced to have completed all work, it is therefore safe to destroy pending resources.
		 */
		void waitDeviceIdle();

	private:
		struct UniqueMaterial;
		using PipelineCache = std::unordered_map<PipelineKey, Pipeline>;
		using WindowList = std::vector<RenderWindow*>;
		using DescriptorSetCacheMap = std::unordered_map<VkDescriptorSetLayout, std::unique_ptr<DescriptorSetCache>>;
		using TextureSet = std::unordered_set<Texture2D*>;
		using BufferSet = std::unordered_set<GPUBuffer*>;
		using VulkanObjectDestructorList = std::vector<VulkanObjectDestructor>;
		using UniqueMaterialCache = std::unordered_map<rtti::TypeInfo, std::unique_ptr<UniqueMaterial>>;

		/**
		 * Binds together all the Vulkan data for a frame.
		 */
		struct Frame
		{
			VkFence								mFence;								///< CPU sync primitive
			std::vector<Texture2D*>				mTextureDownloads;					///< All textures currently being downloaded
			VkCommandBuffer						mUploadCommandBuffer;				///< Command buffer used to upload data from CPU to GPU
			VkCommandBuffer						mDownloadCommandBuffers;			///< Command buffer used to download data from GPU to CPU
			VkCommandBuffer						mHeadlessCommandBuffers;			///< Command buffer used to record operations not associated with a window.
			VulkanObjectDestructorList			mQueuedVulkanObjectDestructors;		///< All Vulkan resources queued for destruction
		};

		/**
		 * Binds together a shader and material.
		 * Used as a sharable shader / material combination.
		 */
		struct UniqueMaterial
		{
			UniqueMaterial() = default;
			UniqueMaterial(std::unique_ptr<Shader> shader, std::unique_ptr<Material> material);
			std::unique_ptr<Shader>		mShader = nullptr;					///< Shader instance linked to material
			std::unique_ptr<Material>	mMaterial = nullptr;				///< Material that links to shader
			bool valid() const;
		};

		bool									mEnableHighDPIMode = true;
		bool									mSampleShadingSupported = false;
		bool									mAnisotropicFilteringSupported = false;
		bool									mWideLinesSupported = false;
		bool									mLargePointsSupported = false;
		uint32									mAnisotropicSamples = 1;
		WindowList								mWindows;												
		SceneService*							mSceneService = nullptr;								
		bool									mIsRenderingFrame = false;
		bool									mCanDestroyVulkanObjectsImmediately = true;
		std::unique_ptr<Texture2D>				mEmptyTexture;
		TextureSet								mTexturesToClear;
		TextureSet								mTexturesToUpload;
		BufferSet								mBuffersToUpload;

		int										mCurrentFrameIndex = 0;
		std::vector<Frame>						mFramesInFlight;
		VkCommandBuffer							mCurrentCommandBuffer = VK_NULL_HANDLE;
		RenderWindow*							mCurrentRenderWindow = nullptr;		

		DescriptorSetCacheMap					mDescriptorSetCaches;
		std::unique_ptr<DescriptorSetAllocator> mDescriptorSetAllocator;

		VkInstance								mInstance = VK_NULL_HANDLE;
		VmaAllocator							mVulkanAllocator = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT				mDebugCallback = VK_NULL_HANDLE;
		PhysicalDevice							mPhysicalDevice;
		VkDevice								mDevice = VK_NULL_HANDLE;
		VkCommandPool							mCommandPool = VK_NULL_HANDLE;
		VkFormat								mDepthFormat = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits					mMaxRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkQueue									mQueue = VK_NULL_HANDLE;
		PipelineCache							mPipelineCache;
		uint32									mAPIVersion = 0;
		bool									mInitialized = false;
		bool									mSDLInitialized = false;
		bool									mShInitialized = false;
		UniqueMaterialCache						mMaterials;
		bool									mHeadless = false;
	};
} // nap



