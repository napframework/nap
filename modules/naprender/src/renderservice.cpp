// Local Includes
#include "renderservice.h"
#include "meshcomponent.h"
#include "rendercomponent.h"
#include "renderwindowcomponent.h"
#include "openglrenderer.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"

// External Includes
#include <nap/core.h>

namespace nap
{
	// Register all types
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
		core.registerType(*this, RTTI_OF(MeshComponent));
		core.registerType(*this, RTTI_OF(RenderWindowComponent));
		core.registerType(*this, RTTI_OF(TransformComponent));
		core.registerType(*this, RTTI_OF(CameraComponent));
	}


	// Occurs when an object registers itself with the service
	void RenderService::objectRegistered(Object& inObject)
	{
		// If we have a render window component and glew hasn't been initialized
		// Initialize glew. Otherwise subsequent render calls will fail
		if (inObject.getTypeInfo().isKindOf(RTTI_OF(RenderWindowComponent)))
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

		// Get all window components
		std::vector<RenderWindowComponent*> windows;
		getObjects<RenderWindowComponent>(windows);
		
		// Trigger update
		for (auto& window : windows)
		{
			window->makeActive();
			window->doUpdate();
		}

		// Collect all transform changes and push
		updateTransforms();

		// Trigger render call
		for (auto& window : windows)
		{
			window->makeActive();
			window->doDraw();
			window->swap();
		}
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
	void RenderService::renderObjects(const CameraComponent& camera)
	{
		// Extract camera projection matrix
		const glm::mat4x4 projection_matrix = camera.getProjectionMatrix();
		
		// Extract camera transform
		nap::TransformComponent* cam_xform = camera.getParent()->getComponent<nap::TransformComponent>();
		if (cam_xform == nullptr)
		{
			assert(false);
			nap::Logger::warn("unable to extract view matrix, camera has no transform component: %s", camera.getName().c_str());
		}
		const glm::mat4x4& view_matrix = cam_xform == nullptr ? identityMatrix : cam_xform->getGlobalTransform();
		
		// Get all render components
		std::vector<nap::RenderableComponent*> render_comps;
		getObjects<nap::RenderableComponent>(render_comps);

		// Draw
		for (auto& comp : render_comps)
		{
			Material* comp_mat = comp->getMaterial();
			if (comp_mat == nullptr)
			{
				nap::Logger::warn("render able object has no material: %s", comp->getName().c_str());
				continue;
			}

			// Get xform component
			nap::Entity* parent_entity = comp->getParent();
			assert(parent_entity != nullptr);
			TransformComponent* xform_comp = parent_entity->getComponent<TransformComponent>();
			
			// Make sure it exists and extract global matrix
			if (xform_comp == nullptr)
			{
				nap::Logger::warn("render able object has no transform: %s", comp->getName().c_str());
			}
			const glm::mat4x4& global_matrix = xform_comp == nullptr ? identityMatrix : xform_comp->getGlobalTransform();

			// Set uniform variables
			comp_mat->setUniformValue<glm::mat4x4>(projectionMatrixUniform, projection_matrix);
			comp_mat->setUniformValue<glm::mat4x4>(viewMatrixUniform, view_matrix);
			comp_mat->setUniformValue<glm::mat4x4>(modelMatrixUniform, global_matrix);

			// Draw
			comp->draw();
		}
	}


	// Set the currently active renderer
	void RenderService::setRenderer(const RTTI::TypeInfo& renderer)
	{
		if (!renderer.isKindOf(RTTI_OF(nap::Renderer)))
		{
			nap::Logger::warn(*this, "unable to add: %s as renderer, object not of type: %s", renderer.getName().c_str(), RTTI_OF(nap::Renderer).getName().c_str());
			return;
		}

		// Shut down existing renderer
		shutdown();

		// Set state
		state = State::Uninitialized;

		// Create new renderer
		nap::Renderer* new_renderer = static_cast<nap::Renderer*>(renderer.createInstance());
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