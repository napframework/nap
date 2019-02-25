#include "emographystressdataviewcomponent.h"
#include "emographystress.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>

// nap::emographystressdataviewcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::StressDataViewComponent)
	//RTTI_PROPERTY("StressData", &nap::emography::StressDataViewComponent::mStressData, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::emographystressdataviewcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::StressDataViewComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		void StressDataViewComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{

		}


		bool StressDataViewComponentInstance::init(utility::ErrorState& errorState)
		{
			if (!RangeDataviewComponentInstance::init(errorState))
				return false;
			return true;
		}


		void StressDataViewComponentInstance::update(double deltaTime)
		{

		}


		void StressDataViewComponentInstance::onQuery()
		{
			// Convert time-stamps
			SystemTimeStamp sys_start = mStartTime.toSystemTime();
			SystemTimeStamp sys_end = mEndTime.toSystemTime();

			// Log some info
			// nap::Logger::info("query start time: %s", DateTime(sys_start).toString().c_str());
			// nap::Logger::info("query end time: %s", DateTime(sys_end).toString().c_str());

			// Get readings
			nap::SystemTimer timer;
			timer.start();
			std::vector<std::unique_ptr<ReadingSummaryBase>> model_readings;
			utility::ErrorState errorState;
			if (!mDataModel->getRange<StressIntensityReading>(sys_start, sys_end, mSampleCount, model_readings, errorState))
				nap::Logger::error(errorState.toString());
			nap::Logger::info("Query took: %d (ms)", (int)timer.getMillis().count());
			timer.reset();

			// Copy to vector
			std::vector<float> stress_readings;
			stress_readings.reserve(model_readings.size());
			for (auto& reading_base : model_readings)
			{
				StressIntensityReadingSummary* reading = rtti_cast<StressIntensityReadingSummary>(reading_base.get());
				stress_readings.emplace_back(reading->mObject.mValue);
			}

			// Create reply
			APIEventPtr reply = std::make_unique<APIEvent>("StressReply", mUUID);

			// Populate event
			reply->addArgument<APILong>("startTime", mStartTime.mTimeStamp);
			reply->addArgument<APILong>("endTime", mEndTime.mTimeStamp);
			reply->addArgument<APIInt>("samples", mSampleCount);

			// Dispatch result
			reply->addArgument<APIFloatArray>("data", std::move(stress_readings));
			mAPIService->dispatchEvent(std::move(reply));
			nap::Logger::info("Query dispatch took: %d (ms)", (int)timer.getMillis().count());
		}
	}
}