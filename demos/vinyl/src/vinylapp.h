#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <perspcameracomponent.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <inputrouter.h>
#include <orthocameracomponent.h>
#include <app.h>
#include <nap/core.h>
#include <color.h>
#include <imguiservice.h>
#include <renderservice.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * This app renders 2 meshes and a background.
	 * You can use the GUI to change the color of the vinyl record
	 * You can change the sleeve image using your favorite paint program and hot-load the changes directly in to
	 * the running application. 
	 * 
	 * This example applies a basic light to both the record and sleeve using the vertex and fragment shader. 
	 * NAP does not include lights by default. 
	 * IMGUI is used to render a GUI panel that you can use to change some properties.
	 *
	 * The meshes are both extracted from 'vinyl_cover.fbx'. Nap automatically detects and converts fbx files.
	 * The output of this conversion is one or multiple binary .mesh file(s). This .mesh file is generated for every individual mesh inside the fbx. 
	 * You can directly link the .mesh objects in to your application using JSON. 
	 * Conversion is performed after compilation when the fbx changed or is new.
	 * Refer to vinylapp.json for the entire app setup
	 */
	class VinylApp : public App
	{
		RTTI_ENABLE(App)
	public:
		VinylApp(nap::Core& core) : nap::App(core)								{ }
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		virtual bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		virtual void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		virtual void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		virtual void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Called when loop finishes
		 */
		virtual int shutdown() override;

		/**
		*	Toggles full screen
		*/
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);

		/**
		*	Called when a window event is received
		*/
		void handleWindowEvent(const WindowEvent& windowEvent);
		
	private:
		
		// Matches background to viewport size
		void positionBackground();

		// Updates camera location
		void setCameraLocation();

		// Updates color of the vinyl record
		void setRecordColor();
		
		ObjectPtr<ImageFromFile>	mVinylLabelImg = nullptr;			//< Vinyl Label Image Resource
		ObjectPtr<ImageFromFile>	mVinylCoverImg = nullptr;			//< Vinyl Cover Image Resource
		
		RenderService*				mRenderService = nullptr;			//< Render Service that handles render calls
		IMGuiService*				mGuiService = nullptr;				//< Gui Service handles all gui related update / drawing
		ResourceManager*			mResourceManager = nullptr;			//< Manages all the loaded resources
		SceneService*				mSceneService = nullptr;			//< Manages all the objects in the scene
		
		ObjectPtr<RenderWindow>		mRenderWindow = nullptr;			//< Pointer to the spawned render window
		ObjectPtr<EntityInstance>	mModelEntity = nullptr;				//< Pointer to the entity that holds all the vinyl parts
		ObjectPtr<EntityInstance>	mCameraEntity = nullptr;			//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance>	mBackgroundEntity = nullptr;		//< Pointer to the entity that holds the background image

		// Color of the vinyl record
		nap::RGBColorFloat			mRecordColor = nap::RGBColorFloat(0.07f, 0.07f, 0.07f);
	};
}