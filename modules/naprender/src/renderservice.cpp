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
	/**
	 * Helper class that can sort RenderableComponents back to front or front to back.
	 */
	class DepthSorter
	{
	public:
		enum class EMode
		{
			FrontToBack,
			BackToFront
		};

		DepthSorter(EMode mode, const glm::mat4x4& viewMatrix) :
			mViewMatrix(viewMatrix),
			mMode(mode)
		{
		}

		bool operator()(const nap::RenderableComponent* objectA, const nap::RenderableComponent* objectB)
		{
			const nap::EntityInstance& entityA = *objectA->getEntity();
			const nap::TransformComponent& transformA = entityA.getComponent<nap::TransformComponent>();
			const glm::mat4 view_space_a = mViewMatrix * transformA.getGlobalTransform();

			const nap::EntityInstance& entityB = *objectB->getEntity();
			const nap::TransformComponent& transformB = entityB.getComponent<nap::TransformComponent>();
			const glm::mat4 view_space_b = mViewMatrix * transformB.getGlobalTransform();

			float a_z = view_space_a[3].z;
			float b_z = view_space_b[3].z;
			if (mMode == EMode::BackToFront)
				return a_z < b_z;
			else
				return a_z > b_z;
		}

	private:
		const glm::mat4x4& mViewMatrix;
		EMode mMode;
	};


	// Register all types
	void RenderService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(RenderableComponent));
		core.registerType(*this, RTTI_OF(RenderableMeshComponent));
		core.registerType(*this, RTTI_OF(RenderWindowComponent));
	}

	// Register all object creation functions
	void RenderService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<WindowResourceCreator>(*this));
	}


	// Creates a new opengl window and assigns it to the component
	// TODO: Add Mutex
	std::unique_ptr<RenderWindow>  RenderService::createWindow(WindowResource& window, utility::ErrorState& errorState)
	{
		assert(mRenderer != nullptr);

		// Get settings
		RenderWindowSettings window_settings;
		window_settings.width		= window.mWidth;
		window_settings.height		= window.mHeight;
		window_settings.borderless	= window.mBorderless;
		window_settings.resizable	= window.mResizable;
		window_settings.title		= window.mTitle;

		std::unique_ptr<RenderWindow> new_window = mRenderer->createRenderWindow(window_settings, errorState);
		if (new_window == nullptr)
			return nullptr;

		// After window creation, make sure the primary window stays active, so that render resource creation always goes to that context
		getPrimaryWindow().makeCurrent();

		return new_window;
	}

	RenderWindow& RenderService::getPrimaryWindow()
	{
		return mRenderer->getPrimaryWindow();
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
	}


	// Emits the draw call
	void RenderService::render()
	{
		update.trigger();

		// Collect all transform changes and push
		updateTransforms();

		draw.trigger();
	}


	void updateTransformsRecursive(EntityInstance& entity, bool parentDirty, const glm::mat4& parentTransform)
	{
		glm::mat4 new_transform = parentTransform;

		bool is_dirty = parentDirty;
		TransformComponent* transform = entity.findComponent<TransformComponent>();
		if (transform && (transform->isDirty() || parentDirty))
		{
			is_dirty = true;
			transform->update(parentTransform);
			new_transform = transform->getGlobalTransform();
		}

		for (EntityInstance* child : entity.getChildren())
			updateTransformsRecursive(*child, is_dirty, new_transform);
	}

	// Updates all transform components
	void RenderService::updateTransforms()
	{
		updateTransformsRecursive(getCore().getService<ResourceManagerService>()->getRootEntity(), false, glm::mat4(1.0f));
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, const CameraComponent& camera)
	{
		// Get all render components
 		std::vector<nap::RenderableComponent*> render_comps;

		for (EntityInstance* entity : getCore().getService<ResourceManagerService>()->getEntities())
			entity->getComponentsOfType<nap::RenderableComponent>(render_comps);

		// Split into front to back and back to front meshes
		std::vector<nap::RenderableComponent*> front_to_back;
		std::vector<nap::RenderableComponent*> back_to_front;

		for (nap::RenderableComponent* component : render_comps)
		{
			nap::RenderableMeshComponent* renderable_mesh = rtti_cast<RenderableMeshComponent>(component);
			if (renderable_mesh != nullptr)
			{
				EBlendMode blend_mode = renderable_mesh->getMaterialInstance().getBlendMode();
				if (blend_mode == EBlendMode::AlphaBlend)
					back_to_front.push_back(component);
				else
					front_to_back.push_back(component);
			}
		}
		
		// Sort front to back and render those first
		DepthSorter front_to_back_sorter(DepthSorter::EMode::FrontToBack, camera.getViewMatrix());
		std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);
		renderObjects(renderTarget, front_to_back, camera);

		// Then sort back to front and render these
		DepthSorter back_to_front_sorter(DepthSorter::EMode::BackToFront, camera.getViewMatrix());
		std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);
		renderObjects(renderTarget, back_to_front, camera);
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
		glm::mat4x4 view_matrix = camera.getViewMatrix();

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

		// Create new renderer
		nap::Renderer* new_renderer = renderer.create<nap::Renderer>();
		mRenderer.reset(new_renderer);

		if (!mRenderer->init(errorState))
			return false;

		return true;
	}
	
	void RenderService::queueResourceForDestruction(std::unique_ptr<opengl::IGLContextResource> resource) 
	{ 
		if (resource != nullptr)
			mGLContextResourcesToDestroy.emplace_back(std::move(resource)); 
	}


	void RenderService::destroyGLContextResources(const std::vector<ObjectPtr<WindowResource>>& renderWindows)
	{
		// If there is anything scheduled, destroy
		if (!mGLContextResourcesToDestroy.empty())
		{
			// Destroy resources for primary window
			getPrimaryWindow().makeCurrent();
			for (auto& resource : mGLContextResourcesToDestroy)
				resource->destroy(getPrimaryWindow().getContext());

			// We go over the windows to make the GL context active, and then destroy 
			// the resources for that context
			for (const ObjectPtr<WindowResource>& render_window : renderWindows)
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
		assert(mRenderer != nullptr);
		mRenderer->shutdown();
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