#pragma once

// Local includes
#include "heightmesh.h"

// Nap includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <app.h>
#include <smoothdamp.h>

namespace nap
{
	/**
	 * Demo application that is called from within the main loop
	 *
	 * Shows a rotating textured sphere in the center of the viewport
	 * You can use the left mouse button to orbit around the object and 
	 * the right mouse button to zoom in on the object
	 * 
	 * This demo uses 3 important modules:
	 * mod_naprender, mod_napinput, mod_napcameracontrol
	 * The sphere is rendered using a simple material that blends 2 colors based on a texture's alpha value
	 * The sphere is positioned by a transform component and rotated along the y axis by a rotate component
	 * The camera is placed away from the origin but can be moved using the mouse. The camera looks at
	 * the sphere and uses that information to orbit around the object.
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component
	 * Refer to the cpp-update() call for more information on handling input
	 */
	class HeightmapApp : public App
	{
		RTTI_ENABLE(App)
	public:
		HeightmapApp(nap::Core& core) : App(core)	{ }

		/**
		 *	Initialize app specific data structures
		 */
		bool init(utility::ErrorState& error) override;
		
		/**
		 *	Update is called before render, performs all the app logic
		 */
		void update(double deltaTime) override;

		/**
		 *	Render is called after update, pushes all renderable objects to the GPU
		 */
		void render() override;

		/**
		 *	Forwards the received window event to the render service
		 */
		void windowMessageReceived(WindowEventPtr windowEvent) override;
		
		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;
		
		/**
		 *	Toggles full screen
		 */
		void setWindowFullscreen(std::string windowIdentifier, bool fullscreen);
		
		/**
		 *	Called when loop finishes
		 */
		void shutdown() override;

	private:
		// Nap Services
		RenderService*			mRenderService = nullptr;			//< Render Service that handles render calls
		ResourceManager*		mResourceManager = nullptr;			//< Manages all the loaded resources
		SceneService*			mSceneService = nullptr;			//< Manages all the objects in the scene
		InputService*			mInputService = nullptr;			//< Input service for processing input
		IMGuiService*			mGuiService = nullptr;				//< Gui service

		ObjectPtr<RenderWindow> mRenderWindow = nullptr;			//< Pointer to the render window
		ObjectPtr<HeightMesh>	mHeightMesh = nullptr;				//< Pointer to the height map mesh
		ObjectPtr<Material>		mNormalsMaterial = nullptr;			//< Material used to draw the normals
		ObjectPtr<Material>		mHeightmapMaterial = nullptr;		//< Material used to draw the heightmap

		ObjectPtr<EntityInstance> mCameraEntity = nullptr;			//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance> mWorldEntity = nullptr;			//< Pointer to the entity that holds the sphere

		// Gui variables
		float	mBlendValue = 1.0f;									//< Height blend value
		int		mSelection = 2;										//< What we want to display to screen
		float	mNormalOpacity = 0.2;								//< Opacity of the normals
		float	mNormalLength = 0.5;								//< Length of the normals on screen
		RGBColorFloat mNormalColor = { 1.0f,1.0f,1.0f };			//< Color of the normal

		// Value Smoother
		math::FloatSmoothOperator mBlendSmoother = { 1.0f, 0.5f };	//< smooths blend value over time to target value

		// Updates gui components
		void updateGui();
	};
}
