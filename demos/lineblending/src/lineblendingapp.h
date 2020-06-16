#pragma once

// Mod nap render includes
#include <renderablemeshcomponent.h>
#include <renderwindow.h>

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <inputservice.h>
#include <imguiservice.h>
#include <renderservice.h>
#include <app.h>
#include <spheremesh.h>
#include <parametergui.h>
#include <parameterservice.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <imagefromfile.h>

namespace nap
{
	using namespace rtti;

	/**
	 * Demo application that is called from within the main loop
	 *
	 * Shows a line that morphs from target A to target B over time based on a blend speed.
	 * The resulting line is projected onto a canvas that is used to define the laser projection bounds.
	 * If you happen to have an etherdream DAC you can configure it in the json file. 
	 * The blend result is sent to the laser over the network. This application also works without a laser attached to your network. 
	 *
	 * This demo uses of a lot of custom components that are defined in mod_lineblending.
	 * The various components automate the line selection process and modulate the line based on a set of conditions.
	 * Take a look at the various components for a better understanding of what they do.
	 * The app contains 3 important entities: a line, a laser, and a camera. The canvas is a child entity of the laser
	 * The canvas entity uses the information from the laser entity to position itself in space and give itself the right dimensions.
	 * The laser DAC is a resource and is referenced by the LaserOutputComponent that lives under the laser entity.
	 * The laser output component translates the line coordinates to laser coordinates and pushes those to the laser DAC resource.
	 * 
	 * Every line is a resource of type 'PolyLine', which in turn is a generic mesh.
	 * The target (output) line is called 'DisplayLine'. This mesh is continuously updated by the various
	 * components that live under the line entity. The 'source' lines are individual resources that the line blend component
	 * interpolates in between. The result of that operation is stored in the display line.
	 * The display line also has normals which are visualized and used to add a displacement effect.
	 *
	 * All the parameters in the GUI can be saved and read from disk as a preset.
	 * The demo uses multiple parameter groups (nap::ParameterGroup) to group parameters together in logical sections.
	 * Every parameter group contains one or multiple parameters of a specific type. The nap::ParameterGUI
	 * renders the group and all of it's children to screen. * Parameters and parameter groups are declared in JSON and are regular resource. 
	 * You can link to parameters directly from other components and resources. 
	 * Refer to the LineNoiseComponent and LineBlendComponent of this application to see how to access and use them.
	 *
	 * Mouse and key events are forwarded to the input service, the input service collects input events
	 * and processes all of them on update. Because NAP does not have a default space (objects can
	 * be rendered in multiple ways), you need to specify what input actually means to the application. 
	 * The input router does that for you. This demo uses the default one that forwards the events to every input component.
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
		ParameterService* mParameterService = nullptr;					//< Manages all parameters in the application
		ResourcePtr<RenderWindow> mRenderWindow;						//< Pointer to the render window		
		ResourcePtr<EntityInstance> mCameraEntity = nullptr;			//< Pointer to the entity that holds the camera
		ResourcePtr<EntityInstance> mLineEntity = nullptr;				//< Pointer to the entity that holds the sphere
		ResourcePtr<EntityInstance> mLaserEntity = nullptr;				//< Pointer to the entity that represents the laser canvas
		ResourcePtr<ParameterGroup> mParameters = nullptr;				//< Pointer to the root parameter group
		ResourcePtr<ParameterFloat> mLineSizeParam = nullptr;			//< Parameter that controls line size
		ResourcePtr<ParameterVec2> mLinePositionParam = nullptr;		//< Parameter that controls the line position
		ResourcePtr<ImageFromFile> mDisplayImage = nullptr;				//< Test image to display in imgui
		ResourcePtr<ImageFromFile> mBrickImage = nullptr;				//< Test image to display in imgui
		RGBAColor8 mTextHighlightColor = { 0xC8, 0x69, 0x69, 0xFF };	//< GUI text highlight color
		std::unique_ptr<ParameterGUI> mParameterGUI = nullptr;			//< Renders the parameters
	};
}