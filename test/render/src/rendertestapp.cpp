#include "rendertestapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <imguiservice.h>
#include <imgui/imgui.h>
#include "uniforminstances.h"

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
		
		getCore().getService<IMGuiService>()->selectWindow(mRenderWindows[0]);

		mTextureRenderTarget		= mResourceManager->findObject<RenderTarget>("PlaneRenderTarget");
		
 		mScene						= mResourceManager->findObject<Scene>("Scene");
		mPigEntity					= mScene->findEntity("PigEntity");
		// 		mRotatingPlaneEntity		= mScene->findEntity("RotatingPlaneEntity");
 		mPlaneEntity				= mScene->findEntity("PlaneEntity");
 		mWorldEntity				= mScene->findEntity("WorldEntity");
 		mCameraEntityLeft			= mScene->findEntity("CameraEntityLeft");
 		mCameraEntityRight			= mScene->findEntity("CameraEntityRight");
 		mSplitCameraEntity			= mScene->findEntity("SplitCameraEntity");
 		mDefaultInputRouter			= mScene->findEntity("DefaultInputRouterEntity");

		
		/*
		// Set render states
		nap::RenderState render_state;
		render_state.mEnableMultiSampling = true;
		render_state.mPointSize = 2.0f;
		render_state.mPolygonMode = opengl::EPolygonMode::Fill;
		mRenderService->setRenderState(render_state);
		*/

		return true;
	}
	
	
	// Called when the window is updating
	void RenderTestApp::update(double deltaTime)
	{/*
		static double timer = 0.0;
		timer += deltaTime;
		if (timer >= 2.5)
		{
			if (mPigEntity == nullptr)
			{
				rtti::ObjectPtr<Entity> entity = mResourceManager->findObject<Entity>("PigEntity");
				utility::ErrorState error_state;
				mPigEntity = mScene->spawn(*entity, error_state);
			}
			else
			{
				mScene->destroy(mPigEntity);
			}
			timer = 0.0;
		}*/

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
// 		nap::IMesh& mesh = mPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMesh();
// 		nap::Mesh* rtti_mesh = rtti_cast<Mesh>(&mesh);
// 		assert(rtti_mesh != nullptr);
// 		const Vec3VertexAttribute& src_position_attribute = rtti_mesh->GetAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
// 		const std::vector<glm::vec3>& src_positions = src_position_attribute.getData();
// 		
// 		// Retrieve destination (instance) mesh data
// 		MeshInstance& mesh_instance = mesh.getMeshInstance();
// 		Vec3VertexAttribute& dst_position_attribute = mesh_instance.getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
// 		std::vector<glm::vec3>& dst_positions = dst_position_attribute.getData();
// 		
// 		// Sine wave over our quad
// 		for (int index = 0; index != src_positions.size() - 1; ++index)
// 		{
// 			float s = sin(mRenderService->getCore().getElapsedTime() + (float)index * 0.2f);
// 			dst_positions[index] = src_positions[index] * glm::vec3(s,s,s);
// 		}
// 		
// 		dst_positions.back() = *dst_positions.begin();
// 		
// 		utility::ErrorState errorState;
// 		if (!mesh_instance.update(errorState))
// 		{
// 			Logger::fatal(errorState.toString());
// 		}

		// 1. Show a simple window.
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug".
// 		{
// 			ImGui::ShowTestWindow(&mShow);
// 		}
	}
	
	
	// Called when the window is going to render
	void RenderTestApp::render()
	{
		// Render window 0
		{
			RenderWindow* render_window = mRenderWindows[0].get();
			render_window->getWindow()->getBackbuffer().setClearColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

			double currentTime = getCore().getElapsedTime();
			float value = (sin(currentTime) + 1.0) * 0.5;

			TransformComponentInstance& transform_component = mPigEntity->getComponent<TransformComponentInstance>();
			RenderableMeshComponentInstance& renderable_mesh_component = mPigEntity->getComponent<RenderableMeshComponentInstance>();
			//renderable_mesh_component.setClipRect(math::Rect(0, 0, render_window->getWidthPixels(), render_window->getHeightPixels()));

			MaterialInstance& material_instance = renderable_mesh_component.getMaterialInstance();
			if ((int)fmodf(currentTime, 4.0f) > 2.0)
			{
				if (material_instance.getBlendMode() == EBlendMode::Opaque)
				{
					rtti::ObjectPtr<ImageFromFile> test_texture = mResourceManager->findObject<ImageFromFile>("TestTexture");
					assert(test_texture != nullptr);
					Sampler2DInstance* pig_texture_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("pigTexture");
					pig_texture_sampler->setTexture(*test_texture);

					material_instance.setBlendMode(EBlendMode::Additive);
				}
			}
			else
			{
				if (material_instance.getBlendMode() == EBlendMode::Additive)
				{
					rtti::ObjectPtr<ImageFromFile> test_texture = mResourceManager->findObject<ImageFromFile>("PigTexture");
					assert(test_texture != nullptr);
					Sampler2DInstance* pig_texture_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("pigTexture");
					pig_texture_sampler->setTexture(*test_texture);

					material_instance.setBlendMode(EBlendMode::Opaque);
				}
			}

			UniformVec4Instance* color = material_instance.getOrCreateUniform("UBO")->getOrCreateUniform<UniformStructArrayInstance>("mData")->getElement(0).getOrCreateUniform<UniformVec4Instance>("mColor");
			color->setValue(glm::vec4(value, 1.0f - value, 1.0f, 1.0f));

			mRenderService->beginFrame();

			if (mRenderService->beginHeadlessRecording())
			{
				std::vector<RenderableComponentInstance*> components_to_render;
				components_to_render.push_back(&mPigEntity->getComponent<RenderableMeshComponentInstance>());

				mTextureRenderTarget->beginRendering();
				mRenderService->renderObjects(*mTextureRenderTarget, mCameraEntityLeft->getComponent<PerspCameraComponentInstance>(), components_to_render);
				mTextureRenderTarget->endRendering();

				mRenderService->endHeadlessRecording();
			}

			if (mRenderService->beginRecording(*render_window))
			{
				glm::mat4 identity = glm::mat4(1.0f);
				transform_component.setTranslate(glm::vec3(0.0f, 0.0f, 0.0f));
				transform_component.update(identity);
					
				IRenderTarget& backbuffer = render_window->getBackbuffer(); 

				backbuffer.beginRendering();

// 				mRenderService->renderObjects(backbuffer, mCameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>());
// 
// 				transform_component.setTranslate(glm::vec3(1.0f, 0.0f, 0.0f));
// 				transform_component.update(identity);
// 				mRenderService->renderObjects(backbuffer, mCameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>());

				// Render output texture to plane
				std::vector<RenderableComponentInstance*> components_to_render;
				components_to_render.push_back(&mPlaneEntity->getComponent<RenderableMeshComponentInstance>());
				components_to_render.push_back(&mPigEntity->getComponent<RenderableMeshComponentInstance>());
				//components_to_render.push_back(&mRotatingPlaneEntity->getComponent<RenderableMeshComponentInstance>());

				//MaterialInstance& plane_material = mPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
				//plane_material.getOrCreateSampler<Sampler2DInstance>("pigTexture").setTexture(mTextureRenderTarget->getColorTexture());
				//plane_material.getOrCreateUniform<UniformTexture2D>("testTexture").setTexture(mTextureRenderTarget->getColorTexture());
				//plane_material.getOrCreateUniform<UniformTexture2D>("pigTexture").setTexture(mTextureRenderTarget->getColorTexture());
				//plane_material.getOrCreateUniform<UniformInt>("mTextureIndex").setValue(0);
				//plane_material.getOrCreateUniform<UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

// 				nap::MaterialInstance& rotating_plane_material = mRotatingPlaneEntity->getComponent<RenderableMeshComponentInstance>().getMaterialInstance();
// 				rotating_plane_material.getOrCreateUniform<UniformTexture2D>("testTexture").setTexture(mTextureRenderTarget->getColorTexture());
// 				rotating_plane_material.getOrCreateUniform<UniformTexture2D>("pigTexture").setTexture(mTextureRenderTarget->getColorTexture());
// 				rotating_plane_material.getOrCreateUniform<UniformInt>("mTextureIndex").setValue(0);
// 				rotating_plane_material.getOrCreateUniform<UniformVec4>("mColor").setValue({ 1.0f, 1.0f, 1.0f, 1.0f });

				//IRenderTarget& backbuffer = render_window->getBackbuffer();
				backbuffer.setClearColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
				mRenderService->renderObjects(backbuffer, mCameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

// 				// Render sphere using split camera with custom projection matrix
 				mSplitCameraEntity->getComponent<PerspCameraComponentInstance>().setGridLocation(0, 0);
 				components_to_render.clear();
 				components_to_render.push_back(&mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>());
 				mRenderService->renderObjects(backbuffer, mSplitCameraEntity->getComponent<PerspCameraComponentInstance>(), components_to_render);

				getCore().getService<IMGuiService>()->draw();

				backbuffer.endRendering();

				mRenderService->endRecording();
			}
		}
	 
		// render window 1
		{
			RenderWindow* render_window = mRenderWindows[1].get();
			
			if (mRenderService->beginRecording(*render_window))
			{
				IRenderTarget& backbuffer = render_window->getBackbuffer();
				backbuffer.beginRendering();

				// Render specific object directly to backbuffer
				std::vector<RenderableComponentInstance*> components_to_render;
				if (mPigEntity != nullptr)
					components_to_render.push_back(&mPigEntity->getComponent<nap::RenderableMeshComponentInstance>());

				mRenderService->renderObjects(backbuffer, mCameraEntityRight->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);
				
				// Render sphere using split camera with custom projection matrix
				mSplitCameraEntity->getComponent<PerspCameraComponentInstance>().setGridLocation(0, 1);
				components_to_render.clear();
				components_to_render.push_back(&mWorldEntity->getComponent<RenderableMeshComponentInstance>());
				mRenderService->renderObjects(backbuffer, mSplitCameraEntity->getComponent<PerspCameraComponentInstance>(), components_to_render);
				

				backbuffer.endRendering();

				mRenderService->endRecording();
			}
		}

		mRenderService->endFrame();
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
