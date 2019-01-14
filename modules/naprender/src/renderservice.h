#pragma once

// External Includes
#include <nap/service.h>
#include <utility/datetime.h>
#include <nap/windowevent.h>
#include <nopengl.h>
#include <thread>
#include <renderablemesh.h>

// Local Includes
#include "renderer.h"
#include "renderstate.h"
#include "vao.h"

namespace opengl
{
	class RenderTarget;
}

namespace nap
{
	// Forward Declares
	class TransformComponentInstance;
	class CameraComponentInstance;
	class RenderableComponentInstance;
	class RenderWindow;
	class RenderService;

	class NAPAPI RenderServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)

	public:
		virtual rtti::TypeInfo getServiceType() override { return RTTI_OF(RenderService); }

		RendererSettings mSettings;		///< Property: 'Settings' All render settings
	};

	/**
	 * Main interface for 2D and 3D rendering operations.
	 * This service initializes the render back-end, manages all vertex array buffers, can be used to create RenderableMesh objects and
	 * provides an interface to render objects to a specific target (screen or back-buffer).
	 * Vertex array object management is handled completely by this service. As a user you only work
	 * with the render interface to render a set of render-able components to a target using a camera.
	 * The service is shut down automatically on exit, and deatroys all windows and left over resources.
	 * When rendering geometry using (most) renderObjects() the service automatically sorts your selection based on the blend mode of the material.
	 * Opaque objects are rendered front to back, alpha blended objects are rendered back to front.
	 */
	class NAPAPI RenderService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using SortFunction = std::function<void(std::vector<RenderableComponentInstance*>&, const CameraComponentInstance&)>;

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

		// Default constructor
		RenderService(ServiceConfiguration* configuration);

		// Default destructor
		virtual ~RenderService();

		/**
		 * Renders all available RenderableComponents in the scene to a specific renderTarget.
		 * The objects to render are sorted using the default sort function (front-to-back for opaque objects, back-to-front for transparent objects).
		 * The sort function is provided by the render service itself, using the default NAP DepthSorter.
		 * @param renderTarget the target to render to
		 * @param camera the camera used for rendering all the available components
		 */
		void renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera);

		/**
		* Renders all available RenderableComponents in the scene to a specific renderTarget.
		*
		* @param renderTarget the target to render to
		* @param camera the camera used for rendering all the available components
		* @param sortFunction The function used to sort the components to render
		*/
		void renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const SortFunction& sortFunction);

		/**
		 * Renders a specific set of objects to a specific renderTarget.
		 * The objects to render are sorted using the default sort function (front-to-back for opaque objects, back-to-front for transparent objects)
		 * The sort function is provided by the render service itself, using the default NAP DepthSorter.
		 * @param renderTarget the target to render to
		 * @param camera the camera used for rendering all the available components
		 * @param comps the components to render to @renderTarget
		 */
		void renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps);

		/**
		* Renders a specific set of objects to a specific renderTarget.
		*
		* @param renderTarget the target to render to
		* @param camera the camera used for rendering all the available components
		* @param comps the components to render to @renderTarget
		* @param sortFunction The function used to sort the components to render
		*/
		void renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps, const SortFunction& sortFunction);

		/**
		* Clears the renderTarget using @flags.
		* @param renderTarget the opengl target to clear
		*/
		void clearRenderTarget(opengl::RenderTarget& renderTarget, opengl::EClearFlags flags);

		/**
		 * Clears all the renderTarget's associated flags (Color, Depth, Stencil)
		 * @param renderTarget the opengl target to clear
		 */
		void clearRenderTarget(opengl::RenderTarget& renderTarget);

		/**
		 * Shuts down the managed renderer
		 */
		virtual void shutdown() override;

		/**
		 * Changes the polygon draw mode for the next set of draw calls. Updates the render state internally.
		 * @param mode the polygon mode to use for the next set of draw calls
		 */
		void setPolygonMode(opengl::EPolygonMode mode);

		/**
		 * Returns the global render state that is used for all OpenGL contexts.
		 * Properties associated with this state (such as the fill mode, point size etc.) are set before rendering a set of objects.
		 * @return the global render state.
		 */
		const RenderState& getRenderState() const																{ return mRenderState; }

		/**
		 * Sets the global render state. This state is used for all OpenGL contexts and settings are pushed before a render call.
		 * You can change settings (such as point-size etc.) at runtime, before rendering objects. 
		 * @param renderState the new state of the renderer to use when rendering objects. 
		 */
		void setRenderState(const RenderState& renderState)														{ mRenderState = renderState; }

		/**
		 * Pushes the global render state to the GPU.
		 * Note that this is called automatically when rendering objects through the render service using RenderObjects().
		 * Use this call when implementing your own draw call in a component.
		 */
		void pushRenderState();

		/**
		 * Batches an OpenGL resource that is dependent on GLContext for destruction, to avoid many GL context switches during destruction.
		 * @param resource: object that is dependent on GL context, that is scheduled for destruction. Notice that ownership is transferred here.
		 */
		void queueResourceForDestruction(std::unique_ptr<opengl::IGLContextResource> resource);

		/**
 		 * Destroys all per-context OpenGL resources that are scheduled for destruction. 
 		 * @param renderWindows: all render windows that are active, as they hold the GL contexts.
		 */
		void destroyGLContextResources(const std::vector<rtti::ObjectPtr<RenderWindow>>& renderWindows);

		/**
		 * Add a new window for the specified resource
		 * @param window the window to add as a valid render target
		 * @param errorState contains the error message if the window could not be added
		 */
		std::shared_ptr<GLWindow> addWindow(RenderWindow& window, utility::ErrorState& errorState);

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
		rtti::ObjectPtr<RenderWindow> findWindow(void* nativeWindow) const;

		/**
		 * Find a RenderWindow based on a window id
		 * @param the associated window id
		 * @return the RenderWindowResource, nullptr if not found
		 */
		rtti::ObjectPtr<RenderWindow> getWindow(uint id) const;

		/**
		 * Get the primary window (i.e. the window that was used to init OpenGL against)
		 */
		GLWindow& getPrimaryWindow();

		/**
		 * @return the id of the primary window
		 */
		const std::string& getPrimaryWindowID() const;

		/**
		 * Add a window event that is processed later, ownership is transferred here
		 * The window number in the event is used to find the right render window to forward the event to
		 * @param event the event to add
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
		* Sets the renderer, the service will own the renderer
		* @param errorState contains the error message if the service could not be initialized
		* @return if the service has been initialized successfully
		*/
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * Updates
		 * @param time in between frames
		 */
		virtual void preUpdate(double deltaTime) override;

		/**
		 *	Process all received messages
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Performs a flush to ensure all recent opengl commands are processed
		 */
		virtual void resourcesLoaded() override;

    private:
		friend class VAOHandle;

		/**
		 * Called by VAOHandle when copied/instantiated.
		 */
		void incrementVAORefCount(const VAOKey& key);

		/**
		* Called by VAOHandle on destruction, decreases refcount and queues VAO for destruction
		* if refcount hits zero.
		*/
		void decrementVAORefCount(const VAOKey& key);

		/**
		 * Holds the currently active renderer
		 */
		std::unique_ptr<nap::Renderer> mRenderer = nullptr;

		/**
		 * Updates the current context's render state by using the latest render state as set by the user.
		 */
		void updateRenderState();

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

		/**
		* Creates a handle to a VertexArrayObject given a material-mesh combination. Internally the RenderService holds a map of VAOs for such
		* combinations, and it hands out reference counted handles to the VAOs that are stored internally. When the refcount of the handle reaches
		* zero, the VAO is removed from the RenderService's map and it will be queued for destruction.
		* @param material: Material to acquire the VAO for.
		* @param meshResource: mesh to acquire the VAO for.
		* @errorstate: in case it was not possible to create a VAO for this combination of material and mesh, this will hold error information.
		* @return On success, this will hold a pointer to the handle, on failure this will return nullptr (check errorState for details).
		*/
		VAOHandle acquireVertexArrayObject(const Material& material, const IMesh& mesh, utility::ErrorState& errorState);

		/**
		* Helper struct to refcount opengl VAOs.
		*/
		struct RefCountedVAO final
		{
			std::unique_ptr<opengl::VertexArrayObject> mObject;
			int mRefCount = 0;
		};

		using ContextSpecificStateMap = std::unordered_map<opengl::GLContext, RenderState>;
		using WindowList = std::vector<RenderWindow*>;
		using VAOMap = std::unordered_map<VAOKey, RefCountedVAO>;

		RenderState	 mRenderState;																//< The latest render state as set by the user
		ContextSpecificStateMap mContextSpecificState;											//< The per-context render state
		std::vector<std::unique_ptr<opengl::IGLContextResource>> mGLContextResourcesToDestroy;	//< Array of per-context GL resources scheduled for destruction
		VAOMap mVAOMap;																			//< Map from material-mesh combination to opengl VAO
		WindowList mWindows;
	};
} // nap



