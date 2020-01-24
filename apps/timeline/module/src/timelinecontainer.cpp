// local includes
#include "TimelineContainer.h"

// external includes

RTTI_BEGIN_CLASS(nap::TimelineContainer)
RTTI_PROPERTY("Timeline File", &nap::TimelineContainer::mTimelineFilePath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool TimelineContainer::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		std::unique_ptr<Timeline> timeline = std::make_unique<Timeline>();

		if (!timeline->init(errorState))
		{
			return false;
		}

		if (!timeline->load(mTimelineFilePath, errorState))
		{
			return false;
		}

		mTimeline = std::move(timeline);

		return true;
	}


	bool TimelineContainer::load(const std::string& name, utility::ErrorState& errorState)
	{
		if (!mTimeline->save(name, errorState))
		{
			return false;
		}

		return true;
	}


	bool TimelineContainer::save(const std::string& name, utility::ErrorState& errorState)
	{
		if (!mTimeline->load(name, errorState))
		{
			return false;
		}

		return true;
	}
}
