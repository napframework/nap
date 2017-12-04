#include "compositioncomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

// nap::compositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CompositionComponent)
	RTTI_PROPERTY("Compositions",	&nap::CompositionComponent::mCompositions,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",			&nap::CompositionComponent::mIndex,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::compositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CompositionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void CompositionComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool CompositionComponentInstance::init(utility::ErrorState& errorState)
	{
		// Make sure there are compositions
		CompositionComponent* resource = getComponent<CompositionComponent>();
		if (errorState.check(resource->mCompositions.empty(), "No compositions found: %s", this->mID.c_str()))
			return false;
		
		// Copy 'm over
		mCompositions.reserve(resource->mCompositions.size());

		for (auto& comp : resource->mCompositions)
		{
			mCompositions.emplace_back(comp.get());
		}

		select(resource->mIndex);
		return true;
	}


	void CompositionComponentInstance::update(double deltaTime)
	{
		if (mCompositionInstance != nullptr)
			mCompositionInstance->update(deltaTime);
	}


	void CompositionComponentInstance::select(int index)
	{
		int sindex = math::clamp<int>(index, 0, mCompositions.size()-1);
		mSelection = mCompositions[sindex];

		mCompositionInstance = std::make_unique<CompositionInstance>(*mSelection);
	}
}