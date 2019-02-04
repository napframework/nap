#pragma once

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <apiservice.h>
#include <app.h>
#include <scene.h>
#include <renderservice.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <inputservice.h>
#include <datamodel.h>
#include "timelinestate.h"

namespace nap
{
	/**
	 * Main application that is called from within the main loop
	 */
	class EmographyApp : public App
	{
		RTTI_ENABLE(App)
	public:
		EmographyApp(nap::Core& core) : App(core)		{ }
		
		/**
		 *	Initialize all the services and app specific data structures
		 */
		bool init(utility::ErrorState& error) override;

		/**
		 *	Update is called every frame
		 */
		void update(double deltaTime) override;
		
		/**
		 * Render is called every frame
		 */
		void render() override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;
	
	private:
		/**
		 * Main function to render the GUI using IMGUI
		 */
		void renderGUI();

		/**
		 * Update the timeline state based on user input
		 */
		void updateTimelineState();

		/**
		 * Render the control panel with controls to clear, generate data, etc
		 */
		void renderControls();

		/**
		 * Render the timeline
		 * @return The height of the renderedtimeline
		 */
		float renderTimeline();
		
		/**
		 * Render the graph from data in the DataModel with the specified height
		 * @param height The height of the graph to render
		 */
		void renderGraph(float height);
		
		/**
		 * Clear all data from the datamodel
		 */
		void clearData();

		/**
		 * Generate data for the specified number of days. The generated data is a sine wave with noise added on top, with periods of no data to simulate 'no activity'
		 * The data is generated starting from the time of the last data in the datamodel, or the current time if the datamodel is empty
		 * @param days The number of days to generate data for
		 */
		void generateData(int days);

		/**
		 *  Forwards the received input event to the input service
		 */
		void inputMessageReceived(InputEventPtr inputEvent) override;

	private:
		// Nap Services
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		RenderService* mRenderService = nullptr;						//< Render Service that handles render calls
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		IMGuiService* mGuiService = nullptr;							//< Service used for updating / drawing guis
		InputService* mInputService = nullptr;							//< Input service for processing input
		rtti::ObjectPtr<RenderWindow> mRenderWindow;					//< Render window
		rtti::ObjectPtr<emography::DataModel> mDataModel = nullptr;		//< The data model containing all data

		bool mMouseWasInsideScreen = false;								//< Whether the mouse was inside the screen during the last update
		emography::TimelineState mTimelineState;						//< The timeline state, used for zooming/panning
		int mResolution = 400;											//< The resolution at which data is returned from the data model (num samples)
		int mGraphYUnits = 100;											//< The max value on the Y axis
		int mNumDaysToGenerate = 7;										//< The number of days to generate data for
	};
}
