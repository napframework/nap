#pragma once

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <app.h>
#include <scene.h>
#include <renderservice.h>
#include <renderwindow.h>
#include <imguiservice.h>
#include <inputservice.h>
#include <datamodel.h>

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

		void render() override;
		
		/**
		 *	Called when loop finishes
		 */
		int shutdown() override;
	
	private:
		void renderGUI();

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
		rtti::ObjectPtr<Scene> mScene = nullptr;						//< Nap scene, contains all entities
		rtti::ObjectPtr<EntityInstance> mController = nullptr;			//< Controlling entity
		rtti::ObjectPtr<EntityInstance> mHistoryEntity = nullptr;		//< History entity
		rtti::ObjectPtr<EntityInstance> mDashboardEntity = nullptr;		//< Dashboard entity
		rtti::ObjectPtr<EntityInstance> mSummaryEntity = nullptr;		//< Summary entity
		
		std::unique_ptr<emography::DataModel> mDataModel;
	};
}
