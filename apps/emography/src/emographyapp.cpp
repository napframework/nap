// Local Includes
#include "emographyapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <inputrouter.h>
#include <emographysnapshot.h>
#include <emographystressdataviewcomponent.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EmographyApp)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap 
{
	using namespace emography;

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

		// Get main entities
		mController = mScene->findEntity("ControllerEntity");
		mSummaryEntity = mScene->findEntity("SummaryEntity");
		mDashboardEntity = mScene->findEntity("DashboardEntity");
		mHistoryEntity = mScene->findEntity("HistoryEntity");

		return true;
	}
	
	
	void EmographyApp::update(double deltaTime)
	{
		// Get current date time
		utility::DateTime now = utility::getCurrentDateTime();

		// Compute time range
		utility::SystemTimeStamp today = now.getTimeStamp();
		utility::SystemTimeStamp yeste = today - std::chrono::hours(24);

		// Set
		StressDataViewComponentInstance& stress_comp = mHistoryEntity->getComponent<StressDataViewComponentInstance>();
		stress_comp.setTimeRange(yeste, today);

		//////////////////////////////////////////////////////////////////////////
		// Date time conversion test
		//////////////////////////////////////////////////////////////////////////
		
		// Create snapshot from datetime
		StressSnapshot snapshot({ nap::emography::EStressState::Over, 1.0f }, now.getTimeStamp());
		
		// Get converted time
		utility::SystemTimeStamp cstamp = snapshot.mTimeStamp.toSystemTime();
		utility::DateTime con(cstamp, utility::DateTime::ConversionMode::Local);
		
		static double time = 0;
		time += deltaTime;
		if (time > 1.0)
		{
			nap::Logger::info("origin: %s", now.toString().c_str());
			nap::Logger::info("conver: %s", con.toString().c_str());
			time = 0.0;
		}
	}

	
	int EmographyApp::shutdown()
	{
		return 0;
	}
}
 
