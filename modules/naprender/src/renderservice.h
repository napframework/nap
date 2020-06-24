#pragma once

// Local Includes
#include "vk_mem_alloc.h"
#include "pipelinekey.h"
#include "renderutils.h"

// External Includes
#include <nap/service.h>
#include <windowevent.h>
#include <rendertarget.h>

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

	/**
	 * Render engine configuration settings
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

		EPhysicalDeviceType			mPreferredGPU = EPhysicalDeviceType::Discrete;					///< Property: 'PreferredGPU' The preferred type of GPU to use. When unavailable, the first GPU in the list is selected. 
		bool						mEnableHighDPIMode = true;										///< Property: 'EnableHighDPI' If high DPI render mode is enabled, on by default
		std::vector<std::string>	mLayers = { "VK_LAYER_LUNARG_standard_validation" };			///< Property: 'Layers' Vulkan Layers that are required to be present in Debug mode. Layers are disabled in release mode.
		std::vector<std::string>	mAdditionalExtensions = { };									///< Property: 'Extensions' Additional required Vulkan device extensions
		bool						mPrintAvailableLayers = false;									///< Property: 'ShowLayers' If all the available Vulkan layers are printed to console
		bool						mPrintAvailableExtensions = false;								///< Property: 'ShowExtensions' If all the available Vulkan extensions are printed to console
		virtual rtti::TypeInfo		getServiceType() override										{ return RTTI_OF(RenderService); }
	};

	/**
	 * Main interface for 2D and 3D rendering operations.
	 * This service initializes the render back-end, manages all vertex array buffers, can be used to create RenderableMesh objects and
	 * provides an interface to render objects to a specific target (screen or back-buffer).
	 * Vertex array object management is handled completely by this service. As a user you only work
	 * with the render interface to render a set of render-able components to a target using a camera.
	 * The service is shut down automatically on exit, and destroys all windows and left over resources.
	 * When rendering geometry using (most) renderObjects() the service automatically sorts your selection based on the blend mode of the material.
	 * Opaque objects are rendered front to back, alpha blended objects are rendered back to front.
	 */
	class NAPAPI RenderService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using SortFunction = std::function<void(std::vector<RenderableComponentInstance*>&, const CameraComponentInstance&)>;
		using VulkanObjectDestructor = std::function<void(RenderService&)>;

		/**
		 * Holds current render state 
		 */
		enum class State : int
		{
			Uninitialized	= -1,		///< The render back end is not initialized
			Initialized		= 0,		///< The render back end initialized correctly
			WindowError		= 1,		///< The render back end produced a window error
			SystemError		= 2,		///< The render back end produced a system error
		};
		
		struct Pipeline
		{
			bool isValid() const
			{
				return mPipeline != nullptr && mLayout != nullptr;
			}

			VkPipeline			mPipeline = nullptr;
			VkPipelineLayout	mLayout = nullptr;
		};

		// Default constructor
		RenderService(ServiceConfiguration* configuration);

		// Default destructor
		virtual ~RenderService();

		void beginFrame();
		void endFrame();

		bool beginHeadlessRecording();
		void endHeadlessRecording();

		bool beginRecording(RenderWindow& renderWindow);
		void endRecording();

		void queueVulkanObjectDestructor(const VulkanObjectDestructor& function);

		/**
		 * Renders all available RenderableComponents in the scene to a specific renderTarget.
		 * The objects to render are sorted using the default sort function (front-to-back for opaque objects, back-to-front for transparent objects).
		 * The sort function is provided by the render service itself, using the default NAP DepthSorter.
		 * Components that can't be rendered with the given camera are omitted. 
		 * @param renderTarget the target to render to
		 * @param camera the camera used for rendering all the available components
		 */
		void renderObjects(IRenderTarget& renderTarget, CameraComponentInstance& camera);

		/**
		* Renders all available RenderableComponents in the scene to a specific renderTarget.
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
		 * Shuts down the managed renderer
		 */
		virtual void shutdown() override;

		/**
		 * Add a new window for the specified resource
		 * @param window the window to add as a valid render target
		 * @param errorState contains the error message if the window could not be added
		 */
		bool addWindow(RenderWindow& window, utility::ErrorState& errorState);

		/**
		 * Remove a window
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
		* Creates a renderable mesh that represents the coupling between a mesh and material that can be rendered to screen.
		* Internally the renderable mesh manages a vertex array object that is issued by the render service.
		* This function should be called from on initialization of components that work with meshes and materials: ie: all types of RenderableComponent. 
		* The result should be validated by calling RenderableMesh.isValid(). Invalid mesh / material representations can't be rendered together.
		* @param mesh The mesh that is used in the mesh-material combination.
		* @param materialInstance The material instance that is used in the mesh-material combination.
		* @param errorState If this function returns an invalid renderable mesh, the error state contains error information.
		* @return A RenderableMesh object that can be used in setMesh calls. Check isValid on the object to see if creation succeeded or failed.
		*/
		RenderableMesh createRenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState);

		Pipeline getOrCreatePipeline(IRenderTarget& renderTarget, IMesh& mesh, MaterialInstance& materialInstance, utility::ErrorState& errorState);

		void removeTextureRequests(Texture2D& texture);
		void requestTextureUpload(Texture2D& texture);
		void requestTextureDownload(Texture2D& texture);

		void requestBufferUpload(GPUBuffer& buffer);
		void removeBufferRequests(GPUBuffer& buffer);

		DescriptorSetCache& getOrCreateDescriptorSetCache(VkDescriptorSetLayout layout);

		VmaAllocator getVulkanAllocator() const { return mVulkanAllocator; }

		int getCurrentFrameIndex() const				{ return mCurrentFrameIndex; }
		VkCommandBuffer getCurrentCommandBuffer()		{ assert(mCurrentCommandBuffer != nullptr); return mCurrentCommandBuffer; }
		RenderWindow* getCurrentRenderWindow()			{ return mCurrentRenderWindow; }

		/**
		 * @return Vulkan runtime instance
		 */
		VkInstance getVulkanInstance() const										{ return mInstance; }

		/**
		 * @return Selected Vulkan compatible hardware device
		 */
		VkPhysicalDevice getPhysicalDevice() const									{ return mPhysicalDevice; }

		/**
		 * @return all supported hardware features
		 */
		const VkPhysicalDeviceFeatures& getPhysicalDeviceFeatures() const			{ return mPhysicalDeviceFeatures; }

		/**
		* @return the version of Vulkan supported by the device
		*/
		uint32_t getPhysicalDeviceVersion() const									{ return mPhysicalDeviceProperties.apiVersion; }

		/**
		 * @return all hardware properties
		 */
		const VkPhysicalDeviceProperties&	getPhysicalDeviceProperties() const		{ return mPhysicalDeviceProperties; }

		/**
		 * Returns the handle to the logical Vulkan device,
		 * represents the hardware together with the extensions, selected queues and features enabled for it.
		 * @return The logical Vulkan device.
		 */
		VkDevice getDevice() const						{ return mDevice; }

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
		 * @return if sample shading is enabled
		 */
		bool sampleShadingSupported() const;

		/**
		 * @return the used depth format.
		 */
		VkFormat getDepthFormat() const { return mDepthFormat; }

		VkCommandPool getCommandPool() const { return mCommandPool; }
		VkImageAspectFlags getDepthAspectFlags() const;
		unsigned int getGraphicsQueueIndex() const { return mGraphicsQueueIndex; }
		VkQueue getGraphicsQueue() const { return mGraphicsQueue; }

		Texture2D& getEmptyTexture() const { return *mEmptyTexture; }

		int getMaxFramesInFlight() const { return 2; }

		void getFormatProperties(VkFormat format, VkFormatProperties& outProperties);

		/**
		 * Called when a new window is added to the system
		 */
		nap::Signal<nap::RenderWindow&> windowAdded;

		/**
		 * Called just before a window is removed from the system
		 */
		nap::Signal<nap::RenderWindow&> windowRemoved;

	protected:
		/**
		* Object creation registration
		*/
		virtual void registerObjectCreators(rtti::Factory& factory) override;

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
		 * Invoked when exiting the main loop, after app shutdown is called
		 * This is called before shutdown() and before the resources are destroyed.
		 * Use this function if your service needs to reset its state before resources 
		 * are destroyed
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void preShutdown();

		/**
		 * Invoked when the resource manager is about to load resources
		 */
		virtual void preResourcesLoaded() override;

		/**
		 * Called before update, ensures the primary window is the active window before update is called.
		 * @param deltaTime time in seconds in between frames.
		 */
		virtual void preUpdate(double deltaTime) override;

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
		 * Processes all window related events for all available windows
		 */
		void processEvents();

		bool initEmptyTexture(nap::utility::ErrorState& errorState);

		void transferData(VkCommandBuffer commandBuffer, const std::function<void()>& transferFunction);
		void downloadData();
		void uploadData();

		void updateTextureDownloads();
		void processVulkanDestructors(int frameIndex);

		void waitDeviceIdle();

	private:
		
		// Using defines, makes handling types easier
		using PipelineCache = std::unordered_map<PipelineKey, Pipeline>;
		using WindowList = std::vector<RenderWindow*>;
		using DescriptorSetCacheMap = std::unordered_map<VkDescriptorSetLayout, std::unique_ptr<DescriptorSetCache>>;
		using TextureSet = std::unordered_set<Texture2D*>;
		using BufferSet = std::unordered_set<GPUBuffer*>;
		using VulkanObjectDestructorList = std::vector<VulkanObjectDestructor>;

		bool									mEnableHighDPIMode = true;
		bool									mSampleShadingSupported = false;

		VmaAllocator							mVulkanAllocator = nullptr;
		WindowList								mWindows;												//< All available windows
		SceneService*							mSceneService = nullptr;								//< Service that manages all the scenes
		
		bool									mIsInRenderFrame = false;
		bool									mCanDestroyVulkanObjectsImmediately = false;

		std::unique_ptr<Texture2D>				mEmptyTexture;
		TextureSet								mTexturesToUpload;
		BufferSet								mBuffersToUpload;

		struct Frame
		{
			VkFence								mFence;
			std::vector<Texture2D*>				mTextureDownloads;
			VkCommandBuffer						mUploadCommandBuffer;
			VkCommandBuffer						mDownloadCommandBuffers;
			VkCommandBuffer						mHeadlessCommandBuffers;
			VulkanObjectDestructorList			mQueuedVulkanObjectDestructors;
		};

		int										mCurrentFrameIndex = 0;
		std::vector<Frame>						mFramesInFlight;
		VkCommandBuffer							mCurrentCommandBuffer = nullptr;
		RenderWindow*							mCurrentRenderWindow = nullptr;		

		DescriptorSetCacheMap					mDescriptorSetCaches;
		std::unique_ptr<DescriptorSetAllocator> mDescriptorSetAllocator;

		VkInstance								mInstance = nullptr;
		VkDebugReportCallbackEXT				mDebugCallback = nullptr;
		VkPhysicalDevice						mPhysicalDevice = nullptr;
		VkPhysicalDeviceFeatures				mPhysicalDeviceFeatures;
		VkPhysicalDeviceProperties				mPhysicalDeviceProperties;
		uint32_t								mPhysicalDeviceVersion = 0;
		VkDevice								mDevice = nullptr;
		VkCommandPool							mCommandPool = nullptr;
		VkFormat								mDepthFormat;
		int										mGraphicsQueueIndex = -1;
		VkSampleCountFlagBits					mMaxRasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		VkQueue									mGraphicsQueue = nullptr;
		PipelineCache							mPipelineCache;
	};
} // nap



