#include "compositioncomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::CompositionCycleMode)
	RTTI_ENUM_VALUE(nap::CompositionCycleMode::Off,			"Off"),
	RTTI_ENUM_VALUE(nap::CompositionCycleMode::Random,		"Random"),
	RTTI_ENUM_VALUE(nap::CompositionCycleMode::Sequence,	"List")
RTTI_END_ENUM

// nap::compositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::CompositionComponent)
	RTTI_PROPERTY("Compositions",	&nap::CompositionComponent::mCompositions,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("CycleMode",		&nap::CompositionComponent::mCycleMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Index",			&nap::CompositionComponent::mIndex,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DurationScale",	&nap::CompositionComponent::mDurationScale,	nap::rtti::EPropertyMetaData::Default)
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
			mCompositions.emplace_back(comp.get());

		// Copy duration scale
		mDurationScale = resource->mDurationScale;

		// Copy mode
		mCycleMode = resource->mCycleMode;

		// Select composition associated with index
		select(resource->mIndex);
		return true;
	}


	void CompositionComponentInstance::update(double deltaTime)
	{
		// Select a new composition
		if (mSwitch)
		{
			switch (mCycleMode)
			{
			case CompositionCycleMode::Random:
			{
				int new_idx = mCurrentIndex;
				while (new_idx == mCurrentIndex)
				{
					new_idx = math::random(0, mCompositions.size() - 1);
				}
				select(new_idx);
				break;
			}
			case CompositionCycleMode::Sequence:
			{
				int new_index = (mCurrentIndex + 1) % static_cast<int>(mCompositions.size());
				select(new_index);
				break;
			}
			case CompositionCycleMode::Off:
				break;
			}
		}

		// Update the current one
		if (mCompositionInstance != nullptr)
		{
			mCompositionInstance->update(deltaTime);
		}
	}


	void CompositionComponentInstance::select(int index)
	{
		int sindex = math::clamp<int>(index, 0, mCompositions.size() - 1);
		mSelection = mCompositions[sindex];

		// Disconnect from a composition if not null
		if (mCompositionInstance != nullptr)
			mCompositionInstance->finished.disconnect(mCompositionFinishedSlot);

		// Create new composition (destructing old one
		mCompositionInstance = std::make_unique<CompositionInstance>(*mSelection);
		mCompositionInstance->setDurationScale(mDurationScale);

		// Connect to finished signal
		mCompositionInstance->finished.connect(mCompositionFinishedSlot);
		mCurrentIndex = index;
		mSwitch = false;
	}


	void CompositionComponentInstance::setDurationScale(float scale)
	{
		mDurationScale = scale;
		assert(mCompositionInstance != nullptr);
		mCompositionInstance->setDurationScale(mDurationScale);
	}


	void CompositionComponentInstance::compositionFinised(CompositionInstance& composition)
	{
		mSwitch = true;
	}
}