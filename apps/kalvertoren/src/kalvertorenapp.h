#pragma once

// Local Includes
#include "firstpersoncontroller.h"
#include "pointlightcomponent.h"
#include "orbitcontroller.h"
#include "cameracomponent.h"
#include "cameracontroller.h"
#include "artnetmeshfromfile.h"

// External Includes
#include <app.h>
#include <sceneservice.h>
#include <renderservice.h>
#include <inputservice.h>
#include <nap/logger.h>
#include <rendertarget.h>
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>
#include <nap/windowresource.h>
#include <inputrouter.h>
#include <entity.h>
#include <artnetcontroller.h>
#include <utility/datetimeutils.h>

namespace nap
{
	class KalvertorenApp : public App
	{
		RTTI_ENABLE(App)
	public:
		KalvertorenApp(Core& core) : App(core)	{ }

		/**
		 *	Initialize the kalvertoren app and it's resources
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Update the kalvertoren app's resources
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Render stuff to screen
		 */
		virtual void render() override;

		/**
		 *	Shut down the application
		 */
		virtual void shutdown() override;

		/**
		 *	Register input events
		 */
		virtual void windowMessageReceived(WindowEventPtr windowEvent) override;

		/**
		 *	Register window events
		 */
		virtual void inputMessageReceived(InputEventPtr inputEvent) override;

		/**
		 *	Updates the gui
		 */
		void updateGui();

		/**
		 *	Sets the current paint method
		 */
		void selectPaintMethod();

		/**
		 *	Sets the current composition cycle mode
		 */
		void selectCompositionCycleMode();

		/**
		 *	Sets the current palette week
		 */
		void selectPaletteWeek();

		/**
		 *	Sets the current palette cycle mode
		 */
		void selectPaletteCycleMode();

		/**
		 *	Sets the current color palette cycle speed
		 */
		void setColorPaletteCycleSpeed(float seconds);

	private:
		// Nap Objects
		nap::RenderService*									renderService = nullptr;
		nap::ResourceManager*								resourceManager = nullptr;
		nap::SceneService*									sceneService = nullptr;
		nap::InputService*									inputService = nullptr;

		nap::ObjectPtr<nap::RenderWindow>					renderWindow;
		nap::ObjectPtr<nap::EntityInstance>					compositionEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					sceneCameraEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					compositionCameraEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					renderCompositionEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					debugDisplayEntity = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					defaultInputRouter = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					lightEntity = nullptr;
		nap::ObjectPtr<nap::Material>						frameMaterial = nullptr;
		nap::ObjectPtr<nap::Material>						vertexMaterial = nullptr;
		nap::ObjectPtr<nap::EntityInstance>					displayEntity = nullptr;

		// GUI
		int													mMeshSelection = 0;
		int													mPaintMode = 2;
		int													mSelectChannel = 0;
		float												mChannelSpeed = 1.0f;
		int													mPaletteSelection = -1;
		int													mCompositionSelection = 0;
		float												mIntensity = 1.0f;
		bool												mShowIndexColors = false;
		float												mDurationScale = 1.0f;
		bool												mCycleColors = false;
		float												mColorCycleTime = 1.0f;
		int													mCompositionCycleMode = 0;
		int													mSelectedWeek = 1;
		int													mColorPaletteCycleMode = 0;
		int													mDay = 0;
		utility::DateTime									mDateTime;
		RGBColor8											mTextColor = { 0xC8, 0x69, 0x69 };

		/**
		 *	Renders debug views to screen
		 */
		void renderDebugViews();

		/**
		 * Called when a window event is received
		 */
		void handleWindowEvent(const WindowEvent& windowEvent);

		/**
		 *	Updates the position of the debug views
		 */
		void positionDebugViews();
	};                                                                               
}