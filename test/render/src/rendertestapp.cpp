#include "rendertestapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <perspcameracomponent.h>
#include <imguiservice.h>
#include <imgui/imgui.h>
#include <imguiutils.h>
#include "uniforminstance.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTestApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

static bool mShow = true;
 
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

		// Get resource manager
		mResourceManager = getCore().getResourceManager();			
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window0"));
		mRenderWindows.push_back(mResourceManager->findObject<RenderWindow>("Window1"));

		mTextureRenderTarget		= mResourceManager->findObject<RenderTarget>("PlaneRenderTarget");
		
 		mScene						= mResourceManager->findObject<Scene>("Scene");
		mPigEntity					= mScene->findEntity("PigEntity");
 		mPlaneEntity				= mScene->findEntity("PlaneEntity");
 		mWorldEntity				= mScene->findEntity("WorldEntity");
 		mCameraEntityLeft			= mScene->findEntity("CameraEntityLeft");
 		mCameraEntityRight			= mScene->findEntity("CameraEntityRight");
 		mSplitCameraEntity			= mScene->findEntity("SplitCameraEntity");
 		mDefaultInputRouter			= mScene->findEntity("DefaultInputRouterEntity");

		return true;
	}
	
	
	// Called when the window is updating
	void RenderTestApp::update(double deltaTime)
	{
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

		// 1. Show demo window in viewport 1.
 		{
			getCore().getService<IMGuiService>()->selectWindow(mRenderWindows[0]);
 			ImGui::ShowDemoWindow(&mShow);
 		}

		// 1. Show metrics window in viewport 2.
		{
			getCore().getService<IMGuiService>()->selectWindow(mRenderWindows[1]);

			// Add some gui elements
			ImGui::Begin("Controls");
			ImGui::Text(getCurrentDateTime().toString().c_str());
			RGBAColorFloat clr = mTextHighColor.convert<RGBAColorFloat>();
			ImGui::TextColored(clr, "left mouse button to rotate, right mouse button to zoom");
			ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

			// Display world texture in GUI
			if (ImGui::CollapsingHeader("Textures"))
			{
				Texture2D& color_texture = *mTextureRenderTarget->mColorTexture;
				float col_width = ImGui::GetColumnWidth();
				float ratio = (float)color_texture.getHeight() / (float)color_texture.getWidth();
				ImGui::Image(color_texture, ImVec2(col_width, col_width * ratio));
				ImGui::Text("Render Texture");
			}
			ImGui::End();
		}
	}
	
	
	// Called when the window is going to render
	void RenderTestApp::render()
	{
		// Render window 0
		{
			RenderWindow* render_window = mRenderWindows[0].get();
			render_window->setClearColor({ 0.0f, 0.0f, 1.0f, 1.0f });

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

				render_window->setClearColor({ 0.0f, 0.0f, 1.0f, 1.0f });
				render_window->beginRendering();

				// Render output texture to plane
				std::vector<RenderableComponentInstance*> components_to_render;
				components_to_render.push_back(&mPlaneEntity->getComponent<RenderableMeshComponentInstance>());
				components_to_render.push_back(&mPigEntity->getComponent<RenderableMeshComponentInstance>());
				mRenderService->renderObjects(*render_window, mCameraEntityLeft->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);

				// Render sphere using split camera with custom projection matrix
 				mSplitCameraEntity->getComponent<PerspCameraComponentInstance>().setGridLocation(0, 0);
 				components_to_render.clear();
 				components_to_render.push_back(&mWorldEntity->getComponent<nap::RenderableMeshComponentInstance>());
 				mRenderService->renderObjects(*render_window, mSplitCameraEntity->getComponent<PerspCameraComponentInstance>(), components_to_render);
				
				getCore().getService<IMGuiService>()->draw();
				render_window->endRendering();
				mRenderService->endRecording();
			}
		}
	 
		// render window 1
		{
			RenderWindow* render_window = mRenderWindows[1].get();
			
			if (mRenderService->beginRecording(*render_window))
			{
				render_window->beginRendering();

				// Render specific object directly to backbuffer
				std::vector<RenderableComponentInstance*> components_to_render;
				if (mPigEntity != nullptr)
					components_to_render.push_back(&mPigEntity->getComponent<nap::RenderableMeshComponentInstance>());

				mRenderService->renderObjects(*render_window, mCameraEntityRight->getComponent<nap::PerspCameraComponentInstance>(), components_to_render);
				
				// Render sphere using split camera with custom projection matrix
				mSplitCameraEntity->getComponent<PerspCameraComponentInstance>().setGridLocation(0, 1);
				components_to_render.clear();
				components_to_render.push_back(&mWorldEntity->getComponent<RenderableMeshComponentInstance>());
				mRenderService->renderObjects(*render_window, mSplitCameraEntity->getComponent<PerspCameraComponentInstance>(), components_to_render);
				
				getCore().getService<IMGuiService>()->draw();

				render_window->endRendering();
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
		mResourceManager->findObject<RenderWindow>(windowIdentifier)->toggleFullscreen();
	}

	
	int RenderTestApp::shutdown() 
	{
		return 0;
	}
}
