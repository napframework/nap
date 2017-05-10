// Local Includes
#include "renderservice.h"
#include "RenderableMeshComponent.h"
#include "rendercomponent.h"
#include "renderwindowcomponent.h"
#include "openglrenderer.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"

// External Includes
#include <nap/core.h>
#include <glm/gtx/matrix_decompose.hpp>

namespace nap
{
	// Register all types
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
		core.registerType(*this, RTTI_OF(RenderableMeshComponent));
		core.registerType(*this, RTTI_OF(RenderWindowComponent));
		core.registerType(*this, RTTI_OF(TransformComponent));
		core.registerType(*this, RTTI_OF(CameraComponent));
	}


	// Occurs when an object registers itself with the service
	void RenderService::objectRegistered(Object& inObject)
	{
		// If we have a render window component and glew hasn't been initialized
		// Initialize glew. Otherwise subsequent render calls will fail
		if (inObject.get_type().is_derived_from(RTTI_OF(RenderWindowComponent)))
		{
			RenderWindowComponent& new_window = static_cast<RenderWindowComponent&>(inObject);
			createWindow(new_window);
		}
	}


	// Creates a new opengl window and assigns it to the component
	// TODO: Add Mutex
	void RenderService::createWindow(RenderWindowComponent& window)
	{
		// Make sure we don't procedeed when errors have been raised before
		if (state > State::Initialized)
		{
			nap::Logger::fatal(*this, "unable to create new window, previous error occurred");
			return;
		}

		// Make sure we have a renderer
		if (mRenderer == nullptr)
		{
			nap::Logger::fatal(*this, "unable to create new window, no associated renderer");
			state = State::SystemError;
			return;
		}

		// Initialize video render 
		if (state == State::Uninitialized)
		{
			if (!mRenderer->preInit())
			{
				nap::Logger::fatal(*this, "unable to initialize renderer");
				state = State::SystemError;
				return;
			}
		}

		// Get settings
		const nap::RenderWindowSettings& window_settings = window.getConstructionSettings();
		nap::RenderWindow* new_window = mRenderer->createRenderWindow(window_settings);
		if (new_window == nullptr)
		{
			nap::Logger::fatal(*this, "unable to create render window and context");
			state = State::WindowError;
			return;
		}
		
		// Set window on window component
		window.mWindow.reset(new_window);

		// Initialize Glew
		if (state == State::Uninitialized)
		{
			if (!mRenderer->postInit())
			{
				state = State::SystemError;
				nap::Logger::fatal(*this, "unable to finalize render initialization process");
				return;
			}
		}

		state = State::Initialized;
	}


	// Finds all top level transforms
	void RenderService::getTopLevelTransforms(Entity* entity, std::vector<TransformComponent*>& xforms)
	{
		// Get xform on current entity
		nap::TransformComponent* xform_comp = entity->getComponent<TransformComponent>();

		// If we found one, add it
		if (xform_comp != nullptr)
		{
			xforms.emplace_back(xform_comp);
			return;
		}

		// If not try it's children
		for (auto& child : entity->getEntities())
		{
			getTopLevelTransforms(child, xforms);
		}
	}


	// Extract view matrix
	void RenderService::getViewMatrix(const nap::CameraComponent& camera, glm::mat4x4& viewMatrix)
	{
		// Set to be identity
		viewMatrix = identityMatrix;

		// Extract camera transform
		nap::TransformComponent* cam_xform = camera.getParent()->getComponent<nap::TransformComponent>();
		if (cam_xform == nullptr)
		{
			assert(false);
			nap::Logger::warn("unable to extract view matrix, camera has no transform component: %s", camera.getName().c_str());
			return;
		}

		// Update
		viewMatrix = cam_xform->getGlobalTransform();

		// Get look at object
		if (!camera.lookAt.isLinked())
		{
			viewMatrix = glm::inverse(viewMatrix);
			return;
		}

		// Extract lookat component
		RenderableComponent* lookat_comp = const_cast<CameraComponent&>(camera).lookAt.getTarget<RenderableComponent>();
		if (lookat_comp == nullptr)
		{
			nap::Logger::warn(camera, "unable to resolve look at target: %s", camera.lookAt.getPath().toString().c_str());
			viewMatrix = glm::inverse(viewMatrix);
			return;
		}

		// Extract xform to look at
		TransformComponent* lookat_xform = lookat_comp->getParent()->getComponent<TransformComponent>();
		if (lookat_xform == nullptr)
		{
			nap::Logger::warn(camera, "unable to resolve object transform for look at object: %s", camera.lookAt.getPath().toString().c_str());
			viewMatrix = glm::inverse(viewMatrix);
			return;
		}

		// Create lookat matrix
		glm::vec3 lookat_pos = lookat_xform->getGlobalTransform()[3];
		glm::vec3 eye_pos = glm::vec3(viewMatrix[3]);
		viewMatrix = glm::lookAt(eye_pos, lookat_pos, glm::vec3(0, 1, 0));
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
	}


	// Emits the draw call
	void RenderService::render()
	{
		if (state == State::Uninitialized)
		{
			nap::Logger::fatal(*this, "unable to execute render call, service is not initialized");
			return;
		}

		if (state > State::Initialized)
		{
			nap::Logger::fatal(*this, "unable to execute render call, internal error occurred: %d", static_cast<int>(state));
			return;
		}

		update.trigger();

		// Collect all transform changes and push
		updateTransforms();

		draw.trigger();
	}


	// Updates all transform components
	void RenderService::updateTransforms()
	{
		std::vector<TransformComponent*> top_xforms;
		getTopLevelTransforms(&(getCore().getRoot()), top_xforms);
		for (auto& xform : top_xforms)
		{
			xform->update();
		}
	}


	// Render all objects in scene graph using specifief camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, const CameraComponent& camera)
	{
		// Get all render components
		std::vector<nap::RenderableComponent*> render_comps;
		getObjects<nap::RenderableComponent>(render_comps);

		renderObjects(renderTarget, render_comps, camera);
	}

	// Updates the current context's render state by using the latest render state as set by the user.
	void RenderService::updateRenderState()
	{
		opengl::GLContext context = opengl::getCurrentContext();
		ContextSpecificStateMap::iterator context_state = mContextSpecificState.find(context);
		if (context_state == mContextSpecificState.end())
		{
			mContextSpecificState.emplace(std::make_pair(context, mRenderState));
			mContextSpecificState[context].force();
		}
		else
		{
			context_state->second.update(mRenderState);
		}
	}

	// Renders all available objects to a specific renderTarget.
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, const std::vector<RenderableComponent*>& comps, const CameraComponent& camera)
	{
		renderTarget.bind();

		updateRenderState();

		// Extract camera projection matrix
		const glm::mat4x4 projection_matrix = camera.getProjectionMatrix();

		// Extract view matrix
		glm::mat4x4 view_matrix;
		getViewMatrix(camera, view_matrix);

		// Draw
		for (auto& comp : comps)
			comp->draw(view_matrix, projection_matrix);

		renderTarget.unbind();
	}

	// Clears the render target.
	void RenderService::clearRenderTarget(opengl::RenderTarget& renderTarget, opengl::EClearFlags flags)
	{
		renderTarget.bind();
		renderTarget.clear(flags);
		renderTarget.unbind();
	}

	// Set the currently active renderer
	void RenderService::setRenderer(const RTTI::TypeInfo& renderer)
	{
		if (!renderer.is_derived_from(RTTI_OF(nap::Renderer)))
		{
			nap::Logger::warn(*this, "unable to add: %s as renderer, object not of type: %s", renderer.get_name().data(), RTTI_OF(nap::Renderer).get_name().data());
			return;
		}

		// Shut down existing renderer
		shutdown();

		// Set state
		state = State::Uninitialized;

		// Create new renderer
		nap::Renderer* new_renderer = renderer.create<nap::Renderer>();
		mRenderer.reset(new_renderer);
	}


	// Shut down renderer
	void RenderService::shutdown()
	{
		if (state == State::Initialized)
		{
			assert(mRenderer != nullptr);
			mRenderer->shutdown();
		}
		state = State::Uninitialized;
	}

} // Renderservice

RTTI_DEFINE(nap::RenderService)