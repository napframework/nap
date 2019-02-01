// Local Includes
#include "emographyapicontrolcomponent.h"

// External Includes
#include <nap/core.h>
#include <entity.h>
#include <entity.h>
#include <nap/logger.h>

// nap::apihandlecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::APIControlComponent)
	RTTI_PROPERTY("StressViewComponent", &nap::emography::APIControlComponent::mStressViewComponent, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::apihandlecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::emography::APIControlComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace emography
	{
		void APIControlComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
		{
			components.emplace_back(RTTI_OF(nap::APIComponent));
		}


		bool APIControlComponentInstance::init(utility::ErrorState& errorState)
		{
			mComponentInstance = getEntityInstance()->findComponent<APIComponentInstance>();
			if (!errorState.check(mComponentInstance != nullptr, "%s: unable to find required API component", this->mID.c_str()))
				return false;

			const nap::APISignature* api_signature = mComponentInstance->findSignature("updateView");
			if (!errorState.check(api_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "updateView"))
				return false;
			mComponentInstance->registerCallback(*api_signature, mUpateViewSlot);

			return true;
		}


		void APIControlComponentInstance::update(double deltaTime)
		{

		}


		void APIControlComponentInstance::updateView(const nap::APIEvent& apiEvent)
		{
			TimeStamp start_time(apiEvent[0].asLong());
			TimeStamp end_time(apiEvent[1].asLong());
			int samples = apiEvent[2].asInt();
			mStressViewComponent->query(start_time, end_time, samples);
		}
	}
}