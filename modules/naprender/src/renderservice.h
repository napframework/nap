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
	* Global render state object. Users can freely change values in this object.
	*
	* Purpose of this object is to:
	* 1) Provide an OpenGL independent way of setting render state
	* 2) Provide a state object that can be duplicated for multiple GL contexts
	* 3) Minimize openGL state changes
	* 
	* RenderService will maintain a global render state, as well as render states per GL context.
	* Whenever objects are rendered, the RenderService will diff the global state against the
	* context's state and update only the GL states that are necessary. 
	*/
	struct RenderState
	{
		bool mEnableDepthTest = true;
		bool mEnableBlending = true;
		bool mEnableMultiSampling = true;
		float mLineWidth = 1.0f;
		float mPointSize = 1.0;
		opengl::PolygonMode mPolygonMode = opengl::PolygonMode::FILL;

	private:
		friend class RenderService;

		/**
		* Forces the setting of all render states as currently set.
		*/
		void Force();

		/**
		* Switches all render states as set in @targetRenderState. Only the renderStates that are different
		will actually cause openGL calls.
		*/
		void Update(const RenderState& targetRenderState);
	};

	/**
	 * Main interface for rendering operations. 
	 */
	class RenderService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)

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
		RenderService();

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
		 * @param renderer the type of renderer to useo
		 */
		void setRenderer(const RTTI::TypeInfo& renderer);

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
		RenderState& GetRenderState() { return mRenderState; }

	protected:
		/**
		 * Type registration
		 */
		virtual void registerTypes(nap::Core& core) override;

		/**
		 * Occurs when an object registers itself with the service
		 */
		virtual void objectRegistered(Object& inObject) override;

    private:

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
		 * Returns the view matrix of the associated camera
		 * The view is determined by a number of factors including the camera's position
		 * and possible look at objects
		 * @param camera: The camera to extract the view matrix for
		 * @param viewMatrix: The populated view matrix
		 */
		void getViewMatrix(const nap::CameraComponent& camera, glm::mat4x4& viewMatrix);

		/**
		* Updates the current context's render state by using the latest render state as set by the user.
		*/
		void UpdateRenderState();

		using ContextSpecificStateMap = std::unordered_map<opengl::GLContext, RenderState>;

		RenderState mRenderState;									//< The latest render state as set by the user
		ContextSpecificStateMap	mContextSpecificState;				//< The per-context render state
	};
} // nap

RTTI_DECLARE(nap::RenderService)
