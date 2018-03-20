#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <app.h>
#include <spheremesh.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * Shows a line that morphs from target A to target B over time based on the blend speed.
	 * The resulting line is projected onto a canvas that is used to define the laser projection bounds.
	 * If you happen to have an etherdream DAC you can configure it in the json file. 
	 * The result is sent to the laser over the network. Ths application also works without a laser attached to your network.
	 * 
	 * This demo uses of a lot of custom components that are defined in mod_lineblending.
	 * The various components automate the line selection process and modulate the line based on a set of conditions.
	 * Take a look at the various components for a better understanding of what they do.
	 * The app contains 3 important entities: a line, a laser, and a camera. The canvas is a child entity of the laser
	 * The canvas uses the laser to position itself in space and give it the right dimensions
	 * The laser DAC is a resource and is referenced by the LaserOutputComponent that lives under the laser entity.
	 * The laser output component translates the line coordinates to laser coordinates and pushes those to the laser dac resource.
	 * 
	 * Every line is a resource of type 'PolyLine', which in turn is a generic mesh.
	 * The target (output) line is called 'DisplayLine'. This mesh is continuously updated by the various
	 * components that live under the line entity. The 'source' lines are individual resources that the line blend component
	 * interpolates in between. The result of that operation is stored in the display line.
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component
	 * Refer to the cpp-update() call for more information on handling input
	 */
	class LineBlendingApp : public App
	{
		RTTI_ENABLE(App)
	public:
		LineBlendingApp(nap::Core& core) : App(core)	{ }

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
		 *	Called when loop finishes
		 */
		int shutdown() override;

	private:
		// Nap Services
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		InputService* mInputService = nullptr;							//< Input service for processing input
		IMGuiService* mGuiService = nullptr;							//< Manages gui related update / draw calls
		ObjectPtr<RenderWindow> mRenderWindow;							//< Pointer to the render window		
		ObjectPtr<EntityInstance> mCameraEntity = nullptr;				//< Pointer to the entity that holds the camera
		ObjectPtr<EntityInstance> mLineEntity = nullptr;				//< Pointer to the entity that holds the sphere
		ObjectPtr<EntityInstance> mLaserEntity = nullptr;				//< Pointer to the entity that represents the laser canvas
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color

		// Colors
		RGBColorFloat mColorTwo = { 0.784f, 0.411f, 0.411f };			//< Line first color	
		RGBColorFloat mColorOne = { 1.0f, 1.0f, 1.0f };					//< Line second color
		float mBlendSpeed = 1.0f;										//< Line blend speed
		float mLineSize = 0.5f;											//< Size of the line (normalized)
		glm::vec2 mLinePosition = { 0.5f, 0.5f };						//< Position of the line relative to canvas
	};
}
