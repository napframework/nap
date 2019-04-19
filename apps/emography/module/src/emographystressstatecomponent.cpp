#include "emographystressstatecomponent.h"
#include "emographystress.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>

// nap::emographystressdataviewcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::StressStateComponent)
	RTTI_PROPERTY("ReplyName", &nap::emography::StressStateComponent::mReplyName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::emographystressdataviewcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::StressStateComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		void StressStateComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{

		}


		bool StressStateComponentInstance::init(utility::ErrorState& errorState)
		{
			if (!RangeDataviewComponentInstance::init(errorState))
				return false;
			mReplyName = getComponent<StressStateComponent>()->mReplyName;
			return true;
		}


		void StressStateComponentInstance::update(double deltaTime)
		{

		}


		void StressStateComponentInstance::onQuery()
		{
			// Convert time-stamps
			SystemTimeStamp sys_start = mStartTime.toSystemTime();
			SystemTimeStamp sys_end = mEndTime.toSystemTime();

			// Get state readings
			nap::SystemTimer timer;
			timer.start();
			std::vector<std::unique_ptr<ReadingSummaryBase>> model_readings;
			utility::ErrorState errorState;
			if (!mDataModel->getRange<StressStateReading>(sys_start, sys_end, mSampleCount, model_readings, errorState))
				nap::Logger::error(errorState.toString());
			nap::Logger::info("Query took: %d (ms)", (int)timer.getMillis().count());
			timer.reset();

			// Copy to vector
			std::vector<int> state_readings;
			state_readings.reserve(model_readings.size());
			
			// We only return the state that is represented most given a certain sample (and therefore time-range)
			// The state is unknown (-1) when there are no states associated with a given sample
			// Optionally you can return 3 arrays with all the given states (under, normal, over)
			for (auto& reading_base : model_readings)
			{
				StressStateReadingSummary* reading = rtti_cast<StressStateReadingSummary>(reading_base.get());
				int current_max = 0;
				EStressState sample_state = EStressState::Unknown;
				for (int i = 0; i < (int)EStressState::Count; i++)
				{
					int state_count = reading->getCount((EStressState)i);
					if (state_count > current_max)
					{
						current_max = state_count;
						sample_state = (EStressState)(i);
					}
				}
				state_readings.emplace_back((int)sample_state);
			}

			// Create reply
			APIEventPtr reply = std::make_unique<APIEvent>(mReplyName, mUUID);

			// Populate event
			reply->addArgument<APILong>("startTime", mStartTime.mTimeStamp);
			reply->addArgument<APILong>("endTime", mEndTime.mTimeStamp);
			reply->addArgument<APIInt>("samples", mSampleCount);

			// Dispatch result
			reply->addArgument<APIIntArray>("data", std::move(state_readings));
			mAPIService->dispatchEvent(std::move(reply));
			nap::Logger::info("Query dispatch took: %d (ms)", (int)timer.getMillis().count());
		}
	}
}