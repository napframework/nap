// Local Includes
#include "renderservice.h"
#include "renderablemeshcomponent.h"
#include "rendercomponent.h"
#include "renderwindow.h"
#include "transformcomponent.h"
#include "cameracomponent.h"
#include "renderglobals.h"
#include "mesh.h"
#include "depthsorter.h"

// External Includes
#include <nap/core.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <rtti/factory.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>
#include <scene.h>

RTTI_BEGIN_CLASS(nap::RenderServiceConfiguration)
	RTTI_PROPERTY("Settings",	&nap::RenderServiceConfiguration::mSettings,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	RenderService::RenderService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	// Register all object creation functions
	void RenderService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<RenderWindowResourceCreator>(*this));
	}


	void RenderService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(SceneService));
	}


	std::shared_ptr<GLWindow> RenderService::addWindow(RenderWindow& window, utility::ErrorState& errorState)
	{
		assert(mRenderer != nullptr);

		// Get settings
		RenderWindowSettings window_settings;
		window_settings.width		= window.mWidth;
		window_settings.height		= window.mHeight;
		window_settings.borderless	= window.mBorderless;
		window_settings.resizable	= window.mResizable;
		window_settings.title		= window.mTitle;
		window_settings.sync		= window.mSync;

		std::shared_ptr<GLWindow> new_window = mRenderer->createRenderWindow(window_settings, window.mID, errorState);
		if (new_window == nullptr)
			return nullptr;

		mWindows.push_back(&window);

		// After window creation, make sure the primary window stays active, so that render resource creation always goes to that context
		getPrimaryWindow().makeCurrent();

		return new_window;
	}


	void RenderService::removeWindow(RenderWindow& window)
	{
		WindowList::iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val) { return val == &window; });
		assert(pos != mWindows.end());
		mWindows.erase(pos);
	}
	

	rtti::ObjectPtr<RenderWindow> RenderService::findWindow(void* nativeWindow) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val) { return val->getWindow()->getNativeWindow() == nativeWindow; });
		if (pos != mWindows.end())
			return *pos;

		return nullptr;
	}


	rtti::ObjectPtr<RenderWindow> RenderService::getWindow(uint id) const
	{
		WindowList::const_iterator pos = std::find_if(mWindows.begin(), mWindows.end(), [&](auto val) { return val->getNumber() == id; });
		if (pos != mWindows.end())
			return *pos;
		return nullptr;
	}


	GLWindow& RenderService::getPrimaryWindow()
	{
		return mRenderer->getPrimaryWindow();
	}


	const std::string& RenderService::getPrimaryWindowID() const
	{
		return mRenderer->getPrimaryWindowID();
	}


	void RenderService::addEvent(WindowEventPtr windowEvent)
	{
        rtti::ObjectPtr<nap::Window> window = getWindow(windowEvent->mWindow);
		assert (window != nullptr);
		window->addEvent(std::move(windowEvent));
	}


	void RenderService::processEvents()
	{
		for (const auto& window : mWindows)
		{
			window->processEvents();
		}
	}


	// Shut down render service
	RenderService::~RenderService()
	{
		shutdown();
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera)
	{
		renderObjects(renderTarget, camera, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	// Render all objects in scene graph using specified camera
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const SortFunction& sortFunction)
	{
		// Get all render components
		std::vector<nap::RenderableComponentInstance*> render_comps;

		SceneService* scene_service = getCore().getService<SceneService>();
		for (Scene* scene : scene_service->getScenes())
			for (EntityInstance* entity : scene->getEntities())
				entity->getComponentsOfType<nap::RenderableComponentInstance>(render_comps);

		// Render these objects
		renderObjects(renderTarget, camera, render_comps, sortFunction);
	}


	void RenderService::sortObjects(std::vector<RenderableComponentInstance*>& comps, const CameraComponentInstance& camera)
	{
		// Split into front to back and back to front meshes
		std::vector<nap::RenderableComponentInstance*> front_to_back;
		front_to_back.reserve(comps.size());
		std::vector<nap::RenderableComponentInstance*> back_to_front;
		back_to_front.reserve(comps.size());

		for (nap::RenderableComponentInstance* component : comps)
		{
			nap::RenderableMeshComponentInstance* renderable_mesh = rtti_cast<RenderableMeshComponentInstance>(component);
			if (renderable_mesh != nullptr)
			{
				nap::RenderableMeshComponentInstance* renderable_mesh = static_cast<RenderableMeshComponentInstance*>(component);
				EBlendMode blend_mode = renderable_mesh->getMaterialInstance().getBlendMode();	
				if (blend_mode == EBlendMode::AlphaBlend)
					back_to_front.emplace_back(component);
				else
					front_to_back.emplace_back(component);
			}
			else
			{
				front_to_back.emplace_back(component);
			}
		}

		// Sort front to back and render those first
		DepthSorter front_to_back_sorter(DepthSorter::EMode::FrontToBack, camera.getViewMatrix());
		std::sort(front_to_back.begin(), front_to_back.end(), front_to_back_sorter);

		// Then sort back to front and render these
		DepthSorter back_to_front_sorter(DepthSorter::EMode::BackToFront, camera.getViewMatrix());
		std::sort(back_to_front.begin(), back_to_front.end(), back_to_front_sorter);

		// concatinate both in to the output
		comps.clear();
		comps.insert(comps.end(), std::make_move_iterator(front_to_back.begin()), std::make_move_iterator(front_to_back.end()));
		comps.insert(comps.end(), std::make_move_iterator(back_to_front.begin()), std::make_move_iterator(back_to_front.end()));
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
	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps)
	{
		renderObjects(renderTarget, camera, comps, std::bind(&RenderService::sortObjects, this, std::placeholders::_1, std::placeholders::_2));
	}


	void RenderService::renderObjects(opengl::RenderTarget& renderTarget, CameraComponentInstance& camera, const std::vector<RenderableComponentInstance*>& comps, const SortFunction& sortFunction)
	{
		// Sort objects to render
		std::vector<RenderableComponentInstance*> components_to_render = comps;
		sortFunction(components_to_render, camera);

		renderTarget.bind();

		// Before we render, we always set aspect ratio. This avoids overly complex
		// responding to various changes in render target sizes.
		camera.setRenderTargetSize(renderTarget.getSize());

		// Make sure we update our render state associated with the current context
		updateRenderState();

		// Extract camera projection matrix
		const glm::mat4x4 projection_matrix = camera.getProjectionMatrix();

		// Extract view matrix
		glm::mat4x4 view_matrix = camera.getViewMatrix();

		// Draw components only when camera is supported
		for (auto& comp : components_to_render)
		{
			if (!comp->isSupported(camera))
			{
				nap::Logger::warn("unable to render component: %s, unsupported camera %s", 
					comp->mID.c_str(), camera.get_type().get_name().to_string().c_str());
				continue;
			}
			comp->draw(view_matrix, projection_matrix);
		}

		renderTarget.unbind();
	}


	// Clears the render target.
	void RenderService::clearRenderTarget(opengl::RenderTarget& renderTarget, opengl::EClearFlags flags)
	{
		renderTarget.bind();
		renderTarget.clear(flags);
		renderTarget.unbind();
	}


	void RenderService::clearRenderTarget(opengl::RenderTarget& renderTarget)
	{
		renderTarget.bind();
		renderTarget.clear(opengl::EClearFlags::Color | opengl::EClearFlags::Depth | opengl::EClearFlags::Stencil);
		renderTarget.unbind();
	}


	// Set the currently active renderer
	bool RenderService::init(nap::utility::ErrorState& errorState)
	{
		std::unique_ptr<Renderer> renderer = std::make_unique<nap::Renderer>();
		if (!renderer->init(getConfiguration<RenderServiceConfiguration>()->mSettings, errorState))
			return false;

		mRenderer = std::move(renderer);

		return true;
	}
	

	void RenderService::preUpdate(double deltaTime)
	{
		getPrimaryWindow().makeCurrent();
	}


	void RenderService::update(double deltaTime)
	{
		processEvents();
	}


	void RenderService::resourcesLoaded()
	{
		opengl::flush();
	}


	void RenderService::setPolygonMode(opengl::EPolygonMode mode)
	{
		mRenderState.mPolygonMode = mode;
	}


	void RenderService::queueResourceForDestruction(std::unique_ptr<opengl::IGLContextResource> resource)
	{ 
		if (resource != nullptr)
			mGLContextResourcesToDestroy.emplace_back(std::move(resource)); 
	}


	void RenderService::destroyGLContextResources(const std::vector<rtti::ObjectPtr<RenderWindow>>& renderWindows)
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
			for (const rtti::ObjectPtr<RenderWindow>& render_window : renderWindows)
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
        // If initializing the renderer failed, mRenderer will be null
        if (mRenderer != nullptr)
		    mRenderer->shutdown();
	}

	/**
	 * Two important things to notice in the internal structure for VAOs:
	 *	1) Internally, a cache of opengl::VertexArrayObjects is created for each mesh-material combination. When retrieving a VAO
	 *	   for an already known mesh-material combination, the VAO is retrieved from the cache.
	 *	2) Ownership of the VAO's does not lie in the RenderService: instead it is shared by all clients. To accomplish this, handles
	 *	   are returned to clients that perform refcounting into the internal RenderService cache. When there are no more references 
	 *	   to a VAO, it is queued for destruction. An important detail to notice is that the RenderService does not store handles 
	 *	   internally, it hands them out when pulling VAOs from the cache or when creating new VAOs.
	 */
	VAOHandle RenderService::acquireVertexArrayObject(const Material& material, const IMesh& mesh, utility::ErrorState& errorState)
	{
		/// Construct a key based on material-mesh, and see if we have a VAO for this combination
		VAOKey key(material, mesh.getMeshInstance());
		VAOMap::iterator kvp = mVAOMap.find(key);
		if (kvp != mVAOMap.end())
			return VAOHandle(*this, key, kvp->second.mObject.get());

		// VAO was not found for this material-mesh combination, create a new one
		RefCountedVAO ref_counted_vao;
		ref_counted_vao.mObject = std::make_unique<opengl::VertexArrayObject>();

		// Use the mapping in the material to bind mesh vertex attrs to shader vertex attrs
		for (auto& kvp : material.getShader()->getShader().getAttributes())
		{
			const opengl::ShaderVertexAttribute* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = material.findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), material.getShader()->mVertPath.c_str(), material.mID.c_str()))
				return VAOHandle();

			const opengl::VertexAttributeBuffer* vertex_buffer = mesh.getMeshInstance().getGPUMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(vertex_buffer != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mesh.mID.c_str()))
				return VAOHandle();

			ref_counted_vao.mObject->addVertexBuffer(shader_vertex_attribute->mLocation, *vertex_buffer);
		}

		auto inserted = mVAOMap.emplace(key, std::move(ref_counted_vao));

		return VAOHandle(*this, key, inserted.first->second.mObject.get());
	}


	void RenderService::incrementVAORefCount(const VAOKey& key)
	{
		VAOMap::iterator pos = mVAOMap.find(key);
		assert(pos != mVAOMap.end()); 

		++pos->second.mRefCount;
	}


	void RenderService::decrementVAORefCount(const VAOKey& key)
	{
		VAOMap::iterator pos = mVAOMap.find(key);
		assert(pos != mVAOMap.end());

		// If this is the last usage of this VAO, queue it for destruction (VAOs need to be destructed per active context,
		// so we defer destruction)
		if (--pos->second.mRefCount == 0)
		{
			queueResourceForDestruction(std::move(pos->second.mObject));
			mVAOMap.erase(pos);
		}
	}

} // Renderservice

