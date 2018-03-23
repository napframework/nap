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
	RTTI_PROPERTY("Monday",			&nap::CompositionComponent::mMonday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Tuesday",		&nap::CompositionComponent::mTuesday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Wednesday",		&nap::CompositionComponent::mWednesday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Thursday",		&nap::CompositionComponent::mThursday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Friday",			&nap::CompositionComponent::mFriday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Saturday",		&nap::CompositionComponent::mSaturday,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Sunday",			&nap::CompositionComponent::mSunday,		nap::rtti::EPropertyMetaData::Required)
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

		mWeekMap[utility::EDay::Monday]		= resource->mMonday.get();
		mWeekMap[utility::EDay::Tuesday]	= resource->mTuesday.get();
		mWeekMap[utility::EDay::Wednesday]	= resource->mWednesday.get();
		mWeekMap[utility::EDay::Thursday]	= resource->mThursday.get();
		mWeekMap[utility::EDay::Friday]		= resource->mFriday.get();
		mWeekMap[utility::EDay::Saturday]	= resource->mSaturday.get();
		mWeekMap[utility::EDay::Sunday]		= resource->mSunday.get();
		
		// Copy duration scale
		mDurationScale = resource->mDurationScale;

		// Copy mode
		mCycleMode = resource->mCycleMode;

		// Select composition associated with index
		select(resource->mIndex, getContainer(getDay()));
		return true;
	}


	void CompositionComponentInstance::update(double deltaTime)
	{
		// Select a new composition
		if (mSwitch)
		{
			CompositionContainer& sample_container = getContainer(getDay());
			switch (mCycleMode)
			{
			case CompositionCycleMode::Random:
			{
				int new_idx = mCurrentIndex;
				while (sample_container.count() > 1 && new_idx == mCurrentIndex)
				{
					new_idx = math::random<int>(0, sample_container.count() - 1);
				}
				select(new_idx, sample_container);
				break;
			}
			case CompositionCycleMode::Sequence:
			{
				int new_index = (mCurrentIndex + 1) % static_cast<int>(sample_container.count());
				select(new_index, sample_container);
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


	void CompositionComponentInstance::select(int index, CompositionContainer& container)
	{
		// Make sure we're always sampling range
		int sindex = math::clamp<int>(index, 0, container.count() - 1);
		
		// Set the currently used container and index
		mCurrentIndex = sindex;
		mCurrentContainer = &container;

		// Disconnect from a composition if not null
		if (mCompositionInstance != nullptr)
			mCompositionInstance->finished.disconnect(mCompositionFinishedSlot);

		// Create new composition (destructing old one
		Composition* new_composition = mCurrentContainer->mCompositions[sindex];
		mCompositionInstance = std::make_unique<CompositionInstance>(*new_composition);
		mCompositionInstance->setDurationScale(mDurationScale);

		// Connect to finished signal
		mCompositionInstance->finished.connect(mCompositionFinishedSlot);
		
		// Notify listeners
		mSelectionChanged.trigger(*this);

		// We've switched
		mSwitch = false;
	}


	void CompositionComponentInstance::select(int index)
	{
		select(index, getContainer(getDay()));
	}


	void CompositionComponentInstance::selectDay(nap::utility::EDay day)
	{
		// Turn manual selection on and store the day
		mDay = day;
		switchMode(EMode::Manual);
	}


	const nap::utility::EDay CompositionComponentInstance::getDay() const
	{
		utility::EDay day = utility::EDay::Unknown;
		switch (mMode)
		{
		case CompositionComponentInstance::EMode::Automatic:
			day = utility::getCurrentDateTime().getDay();
			break;
		case CompositionComponentInstance::EMode::Manual:
			day = mDay;
			break;
		default:
			break;
		}
		assert(day != utility::EDay::Unknown);
		return day;
	}


	const nap::CompositionContainer& CompositionComponentInstance::getContainer(utility::EDay day) const
	{
		auto it = mWeekMap.find(day);
		assert(it != mWeekMap.end());
		return *(it->second);
	}


	nap::CompositionContainer& CompositionComponentInstance::getContainer(utility::EDay day)
	{
		auto it = mWeekMap.find(day);
		assert(it != mWeekMap.end());
		return *(it->second);
	}


	void CompositionComponentInstance::setDurationScale(float scale)
	{
		mDurationScale = scale;
		assert(mCompositionInstance != nullptr);
		mCompositionInstance->setDurationScale(mDurationScale);
	}


	int CompositionComponentInstance::getCount() const
	{
		assert(mCurrentContainer != nullptr); 
		return mCurrentContainer->count();
	}


	void CompositionComponentInstance::switchMode(EMode mode)
	{
		mMode = mode;
		select(mCurrentIndex, getContainer(getDay()));
	}


	void CompositionComponentInstance::compositionFinised(CompositionInstance& composition)
	{
		mSwitch = true;
	}
}