#include "emographystressdataviewcomponent.h"

// External Includes
#include <entity.h>

// nap::emographystressdataviewcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::StressDataViewComponent)
	RTTI_PROPERTY("StressData", &nap::emography::StressDataViewComponent::mStressData, nap::rtti::EPropertyMetaData::Default)
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

			// Copy over stress data
			mStressData = getComponent<StressDataViewComponent>()->mStressData;

			return true;
		}


		void StressDataViewComponentInstance::update(double deltaTime)
		{

		}


		void StressDataViewComponentInstance::settingsChanged()
		{
			// Clear data
			mStressData.clear();

			// Allocate
			mStressData.reserve(getSampleCount());

			// Query and fill -> Push to view
		}
	}
}