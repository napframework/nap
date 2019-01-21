// Local Includes
#include "emographyapp.h"

// Nap includes
#include <nap/core.h>
#include <nap/logger.h>
#include <scene.h>
#include <inputrouter.h>
#include <emographysnapshot.h>
#include <emographystressdataviewcomponent.h>
#include <apiargument.h>
#include <utility/fileutils.h>

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

		// Create API Service
		mAPIService = getCore().getService<APIService>();

		// Initialize all services
		APIArgument float_argument(std::make_unique<APIFloat>(1.0f));

		// Some api argument magic
		std::vector<float> mValues = { 1.0f,2.0f,1.0f };
		APIArgument floav_argument(std::make_unique<APIFloatArray>(std::move(mValues)));
		
		bool is_array = floav_argument.isArray();
		bool is_float_v = floav_argument.getValueType() == RTTI_OF(float);
		
		APIFloatArray float_copy = floav_argument.getCopy<APIFloatArray>();
		std::vector<float>* vector_arr = floav_argument.asArray<float>();

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

		std::unique_ptr<APIEvent> api_event(std::make_unique<APIEvent>("particle"));
		api_event->addArgument<APIFloat>("speed",1.0f);
		api_event->addArgument<APIInt>("drag", 2.0f);
		mAPIService->dispatchEvent(std::move(api_event));

		/////////////////////////////////////////////////////////////////////////
		// API checks
		//////////////////////////////////////////////////////////////////////////

		// Call app from parsed json file that includes signature
		std::string buffer;
		if (!utility::readFileToString("calls.json", mAPIMessageString, error))
			return false;

		return true;
	}
	
	
	void EmographyApp::update(double deltaTime)
	{
		// Get current date time
		DateTime now = getCurrentDateTime();

		// Create snapshot from datetime
		StressSnapshot snapshot({ nap::emography::EStressState::Over, 1.0f }, now.getTimeStamp());
		
		// Get converted time
		SystemTimeStamp cstamp = snapshot.mTimeStamp.toSystemTime();
		DateTime con(cstamp, DateTime::ConversionMode::Local);
		
		static double time = 0;
		time += deltaTime;
		if (time > 1.0)
		{
			nap::Logger::info("fps: %02f", getCore().getFramerate());
			nap::Logger::info("origin: %s", now.toString().c_str());
			nap::Logger::info("conver: %s", con.toString().c_str());

			// Call app from parsed json file that includes signature
			utility::ErrorState error;
			//if (!mAPIService->sendMessage(mAPIMessageString.c_str(), &error))
			//	nap::Logger::warn(error.toString());

			time = 0.0;
		}
	}

	
	int EmographyApp::shutdown()
	{
		return 0;
	}
}
 
