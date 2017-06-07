#pragma once

// External Includes
#include <nap/attribute.h>
#include <nap/service.h>
#include <nap/timer.h>
#include <nopengl.h>
#include <thread>
#include <nwindow.h>

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
	class RenderWindowComponent;
	class TransformComponent;
	class CameraComponent;
	class RenderableComponent;

	/**
	 * Main interface for rendering operations. 
	 */
	class RenderService : public Service
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
		 * Call this in your app loop to emit a render call
		 * Note that this call will gather all available windows
		 * and for every window call it's update and render signals
		 * Subscribe to those signals to update app members and
		 * render objects to a specific target
		 */
		void render();

		/**
		* Call this to update all transform components
		* This call is also called when rendering
		*/
		void updateTransforms();

		/**
		 * Renders all available objects to a specific renderTarget.
		 */
		void renderObjects(opengl::RenderTarget& renderTarget, const CameraComponent& camera);

		/**
		 * Renders a specific set of objects to a specific renderTarget.
		 */
		void renderObjects(opengl::RenderTarget& renderTarget, const std::vector<RenderableComponent*>& comps, const CameraComponent& camera);

		/**
		* Clears the renderTarget.
		*/
		void clearRenderTarget(opengl::RenderTarget& renderTarget, opengl::EClearFlags flags);

		/**
		 * @return if OpenGL has been initialized
		 */
		bool isInitialized() const									{ return state == State::Initialized; }

		/**
		 * Sets the renderer, the service will own the renderer
		 * @param renderer the type of renderer to use
		 */
		bool init(const rtti::TypeInfo& renderer, nap::utility::ErrorState& errorState);

		/**
		 * Shuts down the managed renderer
		 */
		void shutdown();

		/**
		* Render signal, emitted every render iteration
		* Connect to this signal to render objects to the context
		* associated with this window.
		*/
		SignalAttribute draw{ this, "Draw" };

		/**
		* Update signal, emitted before a render operation
		* Connect to this signal to update your scene
		* graph before the render call is emitted
		*/
		SignalAttribute update{ this, "Update" };

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
		void destroyGLContextResources(std::vector<RenderWindowComponent*>& renderWindows);

		/**
		* Creates a handle to a VertexArrayObject given a material-mesh combination. Internally the RenderService holds a map of VAOs for such
		* combinations, and it hands out handles to the VAOs that are stored internally. The handle will release itself on destruction.
		* @param material: Material to acquire the VAO for.
		* @param meshResource: mesh to acquire the VAO for.
		* @errorstate: in case it was not possible to create a VAO for this combination of material and mesh, this will hold error information.
		* @return On success, this will hold a pointer to the handle, on failure this will return nullptr (check errorState for details).
		*/
		std::unique_ptr<VAOHandle> acquireVertexArrayObject(const Material& material, const MeshResource& meshResource, utility::ErrorState& errorState);

	protected:
		/**
		 * Type registration
		 */
		virtual void registerTypes(nap::Core& core) override;

		/**
		* Object creation registration
		*/
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * Occurs when an object registers itself with the service
		 */
		virtual void objectRegistered(Object& inObject) override;

    private:
		friend class VAOHandle;

	
		/**
		* Called by VAOHandle on destruction, decreases refcount and queues VAO for destruction
		* if refcount hits zero.
		*/
		void releaseVertexArrayObject(opengl::VertexArrayObject* vao);

		// Keeps track of glew initialization
		// If there is no active context we can't initialize glew
		// This 
		State state = State::Uninitialized;

		/**
		 * Creates a new window and assigns it to the window component
		 * Note that this call implicitly initializes OpenGL
		 */
		void createWindow(RenderWindowComponent& window);

		/**
		 * Holds the currently active renderer
		 */
		std::unique_ptr<nap::Renderer> mRenderer = nullptr;

		/**
		 * Finds all top level transforms, this is a recursive function
		 * If an entity has a transform it will be considered to be at the top of the chain
		 * if not, all subsequent children will be checked, until the top most level xforms have 
		 * been found
		 * @param entity, entity to check for transform component
		 * @param xforms the total amount of top level transforms
		 */
		void getTopLevelTransforms(Entity* entity, std::vector<TransformComponent*>& xforms);

		/**
		* Updates the current context's render state by using the latest render state as set by the user.
		*/
		void updateRenderState();

		using ContextSpecificStateMap = std::unordered_map<opengl::GLContext, RenderState>;

		RenderState mRenderState;									//< The latest render state as set by the user
		ContextSpecificStateMap	mContextSpecificState;				//< The per-context render state

		std::vector<std::unique_ptr<opengl::IGLContextResource>> mGLContextResourcesToDestroy;	///< Array of per-context GL resources scheduled for destruction

		/**
		* Helper struct to refcount opengl VAOs.
		*/
		struct RefCountedVAO final
		{
			std::unique_ptr<opengl::VertexArrayObject> mObject;
			int mRefCount = 1;
		};

		using VAOMap = std::unordered_map<VAOKey, RefCountedVAO>;
		VAOMap mVAOMap;		///< Map from material-mesh combiantion to opengl VAO
	};
} // nap



