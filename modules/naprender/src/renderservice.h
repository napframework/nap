#pragma once

// External Includes
#include <nap/service.h>
#include <nap/timer.h>
#include <nap/windowevent.h>
#include <nopengl.h>
#include <thread>

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

	/**
	 * Main interface for rendering operations. 
	 */
	class NAPAPI RenderService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		/**
		 * Holds current render state 
		 */
		enum class State : int
		{
			Uninitialized		= -1,
			Initialized			= 0,
			WindowError			= 1,
			SystemError			= 2,
		};

		// Default constructor
		RenderService() = default;

		// Default destructor
		virtual ~RenderService();

		/**
		 * Renders all available objects to a specific renderTarget.
		 */
		void renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera);

		/**
		 * Renders a specific set of objects to a specific renderTarget.
		 */
		void renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps);

		/**
		* Clears the renderTarget.
		*/
		void clearRenderTarget(opengl::RenderTarget& renderTarget, opengl::EClearFlags flags);

		/**
		 * Sets the renderer, the service will own the renderer
		 */
		bool init(nap::utility::ErrorState& errorState);

		/**
		 * Shuts down the managed renderer
		 */
		void shutdown();

		/**
		* Returns global render state. Use the fields in this objects to modify the renderstate.
		*/
		RenderState& getRenderState() { return mRenderState; }

		/**
		 * Batches an OpenGL resource that is dependent on GLContext for destruction, to avoid many GL context switches during destruction.
		 * @param resource: object that is dependent on GL context, that is scheduled for destruction. Notice that ownership is transferred here.
		 */
		void queueResourceForDestruction(std::unique_ptr<opengl::IGLContextResource> resource);

		/**
 		 * Destroys all per-context OpenGL resources that are scheduled for destruction. 
 		 * @param renderWindows: all render windows that are active, as they hold the GL contexts.
		 */
		void destroyGLContextResources(const std::vector<ObjectPtr<RenderWindow>>& renderWindows);

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
		 * Add a new window for the specified resource
		 */
		std::unique_ptr<GLWindow> addWindow(RenderWindow& window, utility::ErrorState& errorState);

		/**
		 * Remove a window
		 */
		void removeWindow(RenderWindow& window);

		/**
		 * Find a RenderWindowResource by its native handle
		 * @param nativeWindow the native window handle (i.e. the SDL_Window pointer)
		 */
		RenderWindow* findWindow(void* nativeWindow) const;

		/**
		 * Find a RenderWindow based on it's id
		 * @param the associated window id
		 * @return the RenderWindowResource, nullptr if not found
		 */
		ObjectPtr<RenderWindow> getWindow(uint id) const;

		/**
		 * Get the primary window (i.e. the window that was used to init OpenGL against)
		 */
		GLWindow& getPrimaryWindow();

		/**
		 * Add a window event that is processed later, ownership is transferred here
		 * The window number in the event is used to find the right render window to forward the event to
		 * @param event the event to add
		 */
		void addEvent(WindowEventPtr windowEvent);

		/**
		 *	Processes all window related events for all available windows
		 */
		void processEvents();

	protected:
		/**
		* Object creation registration
		*/
		virtual void registerObjectCreators(rtti::Factory& factory) override;

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

		using ContextSpecificStateMap = std::unordered_map<opengl::GLContext, RenderState>;
		using WindowList = std::vector<RenderWindow*>;

		RenderState mRenderState;									//< The latest render state as set by the user
		ContextSpecificStateMap	mContextSpecificState;				//< The per-context render state

		std::vector<std::unique_ptr<opengl::IGLContextResource>> mGLContextResourcesToDestroy;	///< Array of per-context GL resources scheduled for destruction

		/**
		 * Helper struct to refcount opengl VAOs.
		 */
		struct RefCountedVAO final
		{
			std::unique_ptr<opengl::VertexArrayObject> mObject;
			int mRefCount = 0;
		};

		using VAOMap = std::unordered_map<VAOKey, RefCountedVAO>;
		VAOMap mVAOMap;												///< Map from material-mesh combination to opengl VAO
		
		WindowList mWindows;
	};
} // nap



