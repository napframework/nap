#include "emographystressdataviewcomponent.h"
#include "emographystress.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

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
			// Create reply
			APIEventPtr reply = std::make_unique<APIEvent>("StressReply");

			// Populate event
			reply->addArgument<APILong>("startTime", mStartTime.mTimeStamp);
			reply->addArgument<APILong>("endTime", mEndTime.mTimeStamp);
			reply->addArgument<APIInt>("samples", mSampleCount);
			
			// Add some random values to test
			std::vector<float> stress_reading(mSampleCount, 0);
			for (int i = 0; i < mSampleCount; i++)
				stress_reading[i] = math::random<float>(0.0f, 1.0f);
			reply->addArgument<APIFloatArray>("data", std::move(stress_reading));

			// Dispatch result
			mAPIService->dispatchEvent(std::move(reply));
		}
	}
}