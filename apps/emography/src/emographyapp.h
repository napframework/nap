#pragma once

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <apiservice.h>
#include <app.h>
#include <scene.h>

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
		 *	Called when loop finishes
		 */
		int shutdown() override;
	
		
	private:
		// Nap Services
		ResourceManager* mResourceManager = nullptr;					//< Manages all the loaded resources
		SceneService* mSceneService = nullptr;							//< Manages all the objects in the scene
		APIService* mAPIService = nullptr;								//< Manages all API calls
		rtti::ObjectPtr<Scene> mScene = nullptr;						//< Nap scene, contains all entities
		rtti::ObjectPtr<EntityInstance> mController = nullptr;			//< Controlling entity
		rtti::ObjectPtr<EntityInstance> mHistoryEntity = nullptr;		//< History entity
		rtti::ObjectPtr<EntityInstance> mDashboardEntity = nullptr;		//< Dashboard entity
		rtti::ObjectPtr<EntityInstance> mSummaryEntity = nullptr;		//< Summary entity
		std::string mAPIMessageString;
	};
}
