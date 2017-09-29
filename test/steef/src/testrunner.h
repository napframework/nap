#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>
#include <sdlinput.h>
#include <sdlwindow.h>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>

namespace nap
{
	class TestRunner
	{
		
	public:
		TestRunner();
		~TestRunner() = default;
		
		bool init(Core& core);
		
		void onUpdate();
		void onRender();

		void handleWindowEvent(const WindowEvent& windowEvent);
		void registerWindowEvent(WindowEventPtr windowEvent);
		void registerInputEvent(InputEventPtr inEvent);
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);
		
		void shutdown();
		
	private:
		
		void updateBackgroundImage();
		void updateShader();
		
		
		ObjectPtr<Image> mVinylLabelImg;
		ObjectPtr<Image> mVinylCoverImg;
		
		RenderService* mRenderService;
		ResourceManagerService*	mResourceManagerService;
		SceneService* mSceneService;
		InputService* mInputService;
		
		ObjectPtr<RenderWindow>	mRenderWindow;
		ObjectPtr<EntityInstance> mModelEntity;
		ObjectPtr<EntityInstance> mCameraEntity;
		ObjectPtr<EntityInstance> mBackgroundEntity;
		
		DefaultInputRouter mInputRouter;
	};
}
