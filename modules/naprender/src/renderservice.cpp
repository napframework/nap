// Local Includes
#include "renderservice.h"
#include "renderablemeshcomponent.h"
#include "rendercomponent.h"
#include "renderwindowcomponent.h"
#include "openglrenderer.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"
#include "meshresource.h"
#include "rtti/factory.h"
#include "nap/resourcemanager.h"

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

	// Register all object creation functions
	void RenderService::registerObjectCreators(rtti::Factory& factory)
	{
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
		//if (!camera.lookAt.isLinked())
		{
			viewMatrix = glm::inverse(viewMatrix);
			return;
		}

// 		// Extract lookat component
// 		RenderableComponent* lookat_comp = const_cast<CameraComponent&>(camera).lookAt.getTarget<RenderableComponent>();
// 		if (lookat_comp == nullptr)
// 		{
// 			nap::Logger::warn(camera, "unable to resolve look at target: %s", camera.lookAt.getPath().toString().c_str());
// 			viewMatrix = glm::inverse(viewMatrix);
// 			return;
// 		}
// 
// 		// Extract xform to look at
// 		TransformComponent* lookat_xform = lookat_comp->getEntity()->getComponent<TransformComponent>();
// 		if (lookat_xform == nullptr)
// 		{
// 			nap::Logger::warn(camera, "unable to resolve object transform for look at object: %s", camera.lookAt.getPath().toString().c_str());
// 			viewMatrix = glm::inverse(viewMatrix);
// 			return;
// 		}
// 
// 		// Create lookat matrix
// 		glm::vec3 lookat_pos = lookat_xform->getGlobalTransform()[3];
// 		glm::vec3 eye_pos = glm::vec3(viewMatrix[3]);
// 		viewMatrix = glm::lookAt(eye_pos, lookat_pos, glm::vec3(0, 1, 0));
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
// 		getObjects<nap::RenderableComponent>(render_comps);

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
	bool RenderService::init(const rtti::TypeInfo& renderer, nap::utility::ErrorState& errorState)
	{
		if (!errorState.check(renderer.is_derived_from(RTTI_OF(nap::Renderer)), "unable to add: %s as renderer, object not of type: %s", renderer.get_name().data(), RTTI_OF(nap::Renderer).get_name().data()))
		{
			return false;
		}

		// Shut down existing renderer
		shutdown();

		// Set state
		state = State::Uninitialized;

		// Create new renderer
		nap::Renderer* new_renderer = renderer.create<nap::Renderer>();
		mRenderer.reset(new_renderer);

		return true;
	}


	void RenderService::queueResourceForDestruction(std::unique_ptr<opengl::IGLContextResource> resource) 
	{ 
		if (resource != nullptr)
			mGLContextResourcesToDestroy.emplace_back(std::move(resource)); 
	}


	void RenderService::destroyGLContextResources(std::vector<RenderWindowComponent*>& renderWindows)
	{
		// If there is anything scheduled, destroy
		if (!mGLContextResourcesToDestroy.empty())
		{
			// We go over the windows to make the GL context active, and then destroy 
			// the resources for that context
			for (RenderWindowComponent* render_window : renderWindows)
			{
				render_window->makeActive();
				for (auto& resource : mGLContextResourcesToDestroy)
					resource->destroy(render_window->getWindow()->getContext());
			}
			mGLContextResourcesToDestroy.clear();
		}
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


	std::unique_ptr<VAOHandle> RenderService::acquireVertexArrayObject(const Material& material, const MeshResource& meshResource, utility::ErrorState& errorState)
	{
		/// Construct a key based on material-mesh, and see if we have a VAO for this combination
		VAOKey key(material, meshResource);
		VAOMap::iterator kvp = mVAOMap.find(key);
		if (kvp != mVAOMap.end())
		{
			// Increase refcount and return handle to our internal opengl object
			++kvp->second.mRefCount;
			return VAOHandle::create(*this, kvp->second.mObject.get());
		}

		// VAO was not found for this material-mesh combination, create a new one
		RefCountedVAO ref_counted_vao;
		ref_counted_vao.mObject = std::make_unique<opengl::VertexArrayObject>();

		// Use the mapping in the material to bind mesh vertex attrs to shader vertex attrs
		for (auto& kvp : material.getShader()->getShader().getAttributes())
		{
			const opengl::VertexAttribute* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader()->mVertPath.c_str(), material.mID.c_str()))
				return nullptr;

			const opengl::VertexAttributeBuffer* vertex_buffer = meshResource.getMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(shader_vertex_attribute != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), meshResource.mID.c_str()))
				return nullptr;

			ref_counted_vao.mObject->addVertexBuffer(shader_vertex_attribute->mLocation, *vertex_buffer);
		}

		auto inserted = mVAOMap.emplace(key, std::move(ref_counted_vao));

		return VAOHandle::create(*this, inserted.first->second.mObject.get());
	}


	void RenderService::releaseVertexArrayObject(opengl::VertexArrayObject* vao)
	{
		// Find the VAO in the map by value
		VAOMap::iterator it = find_if(mVAOMap.begin(), mVAOMap.end(), [&](auto&& kvp) { return kvp.second.mObject.get() == vao; });
		assert(it != mVAOMap.end());

		// If this is the last usage of this VAO, queue it for destruction (VAOs need to be destructed per active context,
		// so we defer destruction)
		if (--it->second.mRefCount == 0)
		{
			queueResourceForDestruction(std::move(it->second.mObject));
			mVAOMap.erase(it);
		}
	}

} // Renderservice

RTTI_DEFINE(nap::RenderService)