// Local Includes
#include "emographyapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <inputrouter.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	/**
	 * Initialize all the resources and instances used for drawing
	 * slowly migrating all functionality to nap
	 */
	bool EmographyApp::init(utility::ErrorState& error)
	{
		// Create render service
		mSceneService	= getCore().getService<SceneService>();

		// Initialize all services

		// Get resource manager service
		mResourceManager = getCore().getResourceManager();

		// Load scene
		if (!mResourceManager->loadFile("emography.json", error)) 
			return false;    

		// Get reference to scene
		mScene = mResourceManager->findObject<Scene>("Scene");

		return true;
	}
	
	
	void EmographyApp::update(double deltaTime)
	{
		// Nothing to update here
	}

	
	int EmographyApp::shutdown()
	{
		return 0;
	}
}
 
