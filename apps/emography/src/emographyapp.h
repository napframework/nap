#pragma once

// Nap includes
#include <nap/resourcemanager.h>
#include <sceneservice.h>
#include <app.h>
#include <scene.h>
#include <emographyreading.h>

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
		rtti::ObjectPtr<Scene> mScene = nullptr;						//< Nap scene, contains all entities
		rtti::ObjectPtr<nap::emography::Reading> mReading = nullptr;
	};
}
