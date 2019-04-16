// Local Includes
#include "emographyapicontrolcomponent.h"

// External Includes
#include <nap/core.h>
#include <entity.h>
#include <entity.h>
#include <nap/logger.h>

// nap::apihandlecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::emography::APIControlComponent)
	RTTI_PROPERTY("StressIntensityComponent",	&nap::emography::APIControlComponent::mStressIntensityComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("StressStateComponent",		&nap::emography::APIControlComponent::mStressStateComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearCacheComponent",		&nap::emography::APIControlComponent::mClearCacheComponent,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PopulateCacheComponent",		&nap::emography::APIControlComponent::mPopulateCacheComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AddStressComponent",			&nap::emography::APIControlComponent::mAddStressComponent,		nap::rtti::EPropertyMetaData::Required)
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

			// Update view (query)
			const nap::APISignature* view_signature = mComponentInstance->findSignature("getStressIntensity");
			if (!errorState.check(view_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "getStressIntensity"))
				return false;
			mComponentInstance->registerCallback(*view_signature, mGetStressIntensitySlot);

			// Get stress state samples
			const nap::APISignature* state_signature = mComponentInstance->findSignature("getStressState");
			if (!errorState.check(state_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "getStressState"))
				return false;
			mComponentInstance->registerCallback(*state_signature, mGetStressStateSlot);

			// Clear database cache
			const nap::APISignature* cache_signature = mComponentInstance->findSignature("clearCache");
			if (!errorState.check(cache_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "clearCache"))
				return false;
			mComponentInstance->registerCallback(*cache_signature, mClearCacheSlot);

			// Populate database cache
			const nap::APISignature* populate_signature = mComponentInstance->findSignature("populateCache");
			if (!errorState.check(populate_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "populateCache"))
				return false;
			mComponentInstance->registerCallback(*populate_signature, mPopulateCacheSlot);

			// Populate database caches with settings
			const nap::APISignature* populate_param_signature = mComponentInstance->findSignature("populateCacheParameterized");
			if (!errorState.check(populate_param_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "populateCacheParameterized"))
				return false;
			mComponentInstance->registerCallback(*populate_param_signature, mPopulateCacheParamSlot);

			// Add stress sample
			const nap::APISignature* add_stress_sample_signature = mComponentInstance->findSignature("addSample");
			if (!errorState.check(add_stress_sample_signature != nullptr, "%s: unable to find method with signature: %s", this->mID.c_str(), "addSample"))
				return false;
			mComponentInstance->registerCallback(*add_stress_sample_signature, mAddStressSlot);

			return true;
			}


		void APIControlComponentInstance::update(double deltaTime)
		{

		}


		void APIControlComponentInstance::getStressIntensity(const nap::APIEvent& apiEvent)
		{
			TimeStamp start_time(apiEvent[0].asLong());
			TimeStamp end_time(apiEvent[1].asLong());
			int samples = apiEvent[2].asInt();
			mStressIntensityComponent->query(start_time, end_time, samples, apiEvent.getID());
		}


		void APIControlComponentInstance::getStressState(const nap::APIEvent& apiEvent)
		{
			TimeStamp start_time(apiEvent[0].asLong());
			TimeStamp end_time(apiEvent[1].asLong());
			int samples = apiEvent[2].asInt();
			mStressStateComponent->query(start_time, end_time, samples, apiEvent.getID());
		}

		void APIControlComponentInstance::clearCache(const nap::APIEvent& apiEvent)
		{
			nap::utility::ErrorState error;
			if (!mClearCacheComponent->clearCache(error))
				nap::Logger::error(error.toString());
		}


		void APIControlComponentInstance::populateCache(const nap::APIEvent& apiEvent)
		{
			nap::utility::ErrorState error;
			if (!mPopulateCacheComponent->populate(error))
				nap::Logger::error(error.toString());
		}


		void APIControlComponentInstance::populateCacheParam(const nap::APIEvent& apiEvent)
		{
			nap::utility::ErrorState error;
			mPopulateCacheComponent->setParameters(apiEvent[0].asInt(), apiEvent[1].asInt());
			if (!mPopulateCacheComponent->populate(error))
				nap::Logger::error(error.toString());
		}


		void APIControlComponentInstance::addStressSample(const nap::APIEvent& apiEvent)
		{
			nap::utility::ErrorState error;
			TimeStamp time(apiEvent[0].asLong());
			if (!mAddStressComponent->addSample(time, apiEvent[1].asFloat(), apiEvent[2].asInt(), error))
			    nap::Logger::error(error.toString());
		}
	}
}