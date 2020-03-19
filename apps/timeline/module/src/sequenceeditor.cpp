// local includes
#include "sequenceeditor.h"

// external includes

RTTI_BEGIN_CLASS(nap::SequenceEditor)
RTTI_PROPERTY("Timeline File", &nap::SequenceEditor::mTimelineFilePath, nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	/*
	bool SequenceEditor::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		std::unique_ptr<Sequence> timeline = std::make_unique<Sequence>();

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


	bool SequenceEditor::load(const std::string& name, utility::ErrorState& errorState)
	{
		if (!mTimeline->load(name, errorState))
		{
			return false;
		}

		return true;
	}


	bool SequenceEditor::save(const std::string& name, utility::ErrorState& errorState)
	{
		if (!mTimeline->save(name, errorState))
		{
			return false;
		}

		return true;
	}
	*/
}
