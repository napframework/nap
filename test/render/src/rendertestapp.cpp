#include "rendertestapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <imguiservice.h>
#include <imgui/imgui.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

glm::vec3 mColor{0.0f,0.0f,0.0f};
int mWhite(0);
bool mShow = true;

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool RenderTestApp::init(utility::ErrorState& error)
	{		
		// Create render service
		mRenderService = getCore().getService<RenderService>();		
		mInputService  = getCore().getService<InputService>();
		mSceneService  = getCore().getService<SceneService>();

		// Get resource manager and load
		mResourceManager = getCore().getResourceManager();
		if (!mResourceManager->loadFile("render.json", error))
		{
			Logger::fatal("Unable to deserialize resources: \n %s", error.toString().c_str());
			return false;
		}
		
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window0"));
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window1"));
		
		mTextureRenderTarget		= mResourceManager->findObject<RenderTarget>("PlaneRenderTarget");
		
		mScene						= mResourceManager->findObject<Scene>("Scene");
		mRotatingPlaneEntity		= mScene->findEntity("RotatingPlaneEntity");
		mPlaneEntity				= mScene->findEntity("PlaneEntity");
		mWorldEntity				= mScene->findEntity("WorldEntity");
		mCameraEntityLeft			= mScene->findEntity("CameraEntityLeft");
		mCameraEntityRight			= mScene->findEntity("CameraEntityRight");
		mSplitCameraEntity			= mScene->findEntity("SplitCameraEntity");
		mDefaultInputRouter			= mScene->findEntity("DefaultInputRouterEntity");
		
		// Set render states
		nap::RenderState render_state;
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;
		mRenderService->setRenderState(render_state);

		return true;
	}
	
	
	// Called when the window is updating
	void RenderTestApp::update(double deltaTime)
	{
		static double timer = 0.0;
		timer += deltaTime;
		if (timer >= 2.5)
		{
			if (mPigEntity == nullptr)
			{
				rtti::ObjectPtr<Entity> entity = mResourceManager->findObject<Entity>("PigEntity");
				utility::ErrorState error_state;
				mPigEntity = mScene->spawn(*entity, { }, error_state);
			}
			else
			{
				mScene->destroy(mPigEntity);
			}
			timer = 0.0;
		}

		DefaultInputRouter& input_router = mDefaultInputRouter->getComponent<DefaultInputRouterComponentInstance>().mInputRouter;
		{
			// Update input for first window
			std::vector<nap::EntityInstance*> entities;
			entities.push_back(mCameraEntityLeft.get());
			
			Window* window = mRenderWindows[0].get();
			mInputService->processWindowEvents(*window, input_router, entities);
		}
		
		{
			// Update input for second window
			std::vector<nap::EntityInstance*> entities;
			entities.push_back(mCameraEntityRight.get());
			
			Window* window = mRenderWindows[1].get();
			mInputService->processWindowEvents(*window, input_router, entities);
		}
		
		// Retrieve source (resource) mesh data
		nap::IMesh& mesh = mPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMesh();
		nap::Mesh* rtti_mesh = rtti_cast<Mesh>(&mesh);
		assert(rtti_mesh != nullptr);
		const Vec3VertexAttribute& src_position_attribute = rtti_mesh->GetAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		const std::vector<glm::vec3>& src_positions = src_position_attribute.getData();
		
		// Retrieve destination (instance) mesh data
		MeshInstance& mesh_instance = mesh.getMeshInstance();
		Vec3VertexAttribute& dst_position_attribute = mesh_instance.getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		std::vector<glm::vec3>& dst_positions = dst_position_attribute.getData();
		
		// Sine wave over our quad
		for (int index = 0; index != src_positions.size() - 1; ++index)
		{
			float s = sin(mRenderService->getCore().getElapsedTime() + (float)index * 0.2f);
			dst_positions[index] = src_positions[index] * glm::vec3(s,s,s);
		}
		
		dst_positions.back() = *dst_positions.begin();
		
		utility::ErrorState errorState;
		if (!mesh_instance.update(errorState))
		{
			Logger::fatal(errorState.toString());
		}

		// 1. Show a simple window.
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug".
		{
			ImGui::ShowTestWindow(&mShow);
		}
	}
	
	
	// Called when the window is going to render
	void RenderTestApp::render()
	{
		mRenderService->destroyGLContextResources({ mRenderWindows[0].get(), mRenderWindows[1].get() });
		
		// Render offscreen surface(s)
		{
			mRenderService->getPrimaryWindow().makeCurrent();
			
			// Render entire scene to texture
			mRenderService->clearRenderTarget(mTextureRenderTarget->getTarget());
			mRenderService->renderObjects(mTextureRenderTarget->getTarget(), mCameraEntityLeft->getComponent<PerspCameraComponentInstance>());
		}
		
		
		// Render window 0
		{
			RenderWindow* render_window = mRenderWindows[0].get();
			
			render_window->makeActive();
			
			// Render output texture to plane
			std::vector<RenderableComponentInstance*> components_to_render;
			components_to_render.push_back(&mPlaneEntity->getComponent<RenderableMeshComponentInstance>());
			components_to_render.push_back(&mRotatingPlaneEntity->getComponent<RenderableMeshComponentInstance>());
			
			MaterialInstance& plane_material = mPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
			plane_material.getOrCreateUniform<UniformTexture2D>("testTexture").setTexture(mTextureRenderTarget->getColorTexture());
			plane_material.getOrCreateUniform<UniformTexture2D>("pigTexture").setTexture(mTextureRenderTarget->getColorTexture());
			plane_material.getOrCreateUniform<UniformInt>("mTextureIndex").setValue(0);
			plane_material.getOrCreateUniform<UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
			
			nap::MaterialInstance& rotating_plane_material = mRotatingPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
			rotating_plane_material.getOrCreateUniform<UniformTexture2D>("testTexture").setTexture(mTextureRenderTarget->getColorTexture());
			rotating_plane_material.getOrCreateUniform<UniformTexture2D>("pigTexture").setTexture(mTextureRenderTarget->getColorTexture());
			rotating_plane_material.getOrCreateUniform<UniformInt>("mTextureIndex").setValue(0);
			rotating_plane_material.getOrCreateUniform<UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });
			
			opengl::RenderTarget& backbuffer = render_window->getBackbuffer();
			backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
			mRenderService->clearRenderTarget(backbuffer);
			mRenderService->renderObjects(backbuffer, mCameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);
			
			// Render sphere using split camera with custom projection matrix
			mSplitCameraEntity->getComponent<PerspCameraComponentInstance>().setGridLocation(0, 0);
			components_to_render.clear();
			components_to_render.push_back(&mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>());
			mRenderService->renderObjects(backbuffer, mSplitCameraEntity->getComponent<PerspCameraComponentInstance>(), components_to_render);

			getCore().getService<IMGuiService>()->draw();

			render_window->swap();
		}
	 
		// render window 1
		{
			RenderWindow* render_window = mRenderWindows[1].get();
			
			render_window->makeActive();
			
			// Render specific object directly to backbuffer
			std::vector<RenderableComponentInstance*> components_to_render;
			if (mPigEntity != nullptr)
				components_to_render.push_back(&mPigEntity->getComponent<nap::RenderableMeshComponentInstance>());
			
			opengl::RenderTarget& backbuffer = render_window->getBackbuffer();
			mRenderService->clearRenderTarget(backbuffer, opengl::EClearFlags::Color | opengl::EClearFlags::Depth | opengl::EClearFlags::Stencil);
			mRenderService->renderObjects(backbuffer, mCameraEntityRight->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);
			
			// Render sphere using split camera with custom projection matrix
			mSplitCameraEntity->getComponent<PerspCameraComponentInstance>().setGridLocation(0, 1);
			components_to_render.clear();
			components_to_render.push_back(&mWorldEntity->getComponent<RenderableMeshComponentInstance>());
			mRenderService->renderObjects(backbuffer, mSplitCameraEntity->getComponent<PerspCameraComponentInstance>(), components_to_render);

			render_window->swap(); 
		}
	}
	

	/**
	 * Handles the window event
	 */
	void RenderTestApp::handleWindowEvent(const WindowEvent& windowEvent)
	{
		
	}
	
	
	void RenderTestApp::windowMessageReceived(WindowEventPtr windowEvent)
	{
		mRenderService->addEvent(std::move(windowEvent));
	}
	
	
	void RenderTestApp::inputMessageReceived(InputEventPtr inputEvent)
	{
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

		}
		mInputService->addEvent(std::move(inputEvent));
	}

	
	void RenderTestApp::setWindowFullscreen(std::string windowIdentifier, bool fullscreen) 
	{
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->getWindow()->setFullScreen(fullscreen);
	}

	
	int RenderTestApp::shutdown() 
	{
		return 0;
	}
}
