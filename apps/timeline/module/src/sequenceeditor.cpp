// local includes
#include "sequenceeditor.h"

// external includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::SequenceEditor)
RTTI_PROPERTY("Sequence Player", &nap::SequenceEditor::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	
	bool SequenceEditor::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		// define slots
		mSegmentDurationChangeSlot = Slot<const SequenceTrack&, const SequenceTrackSegment&, float> ( [this](const SequenceTrack& track, const SequenceTrackSegment& segment, float amount) 
		{
			segmentDurationChange(track, segment, amount);
		} );

		mSaveSlot = Slot<>([this]()
		{
			save();
		});

		return true;
	}


	void SequenceEditor::registerView(SequenceEditorView* sequenceEditorView)
	{
		bool found = (std::find(mViews.begin(), mViews.end(), sequenceEditorView) != mViews.end());

		if (!found)
		{
			mViews.emplace_back(sequenceEditorView);

			sequenceEditorView->registerSegmentDurationChangeSlot(mSegmentDurationChangeSlot);
			sequenceEditorView->registerSaveSlot(mSaveSlot);
		}
	}


	void SequenceEditor::unregisterView(SequenceEditorView* sequenceEditorView)
	{
		bool found = (std::find(mViews.begin(), mViews.end(), sequenceEditorView) != mViews.end());

		if (found)
		{
			mViews.remove(sequenceEditorView);

			sequenceEditorView->unregisterSegmentDurationChangeSlot(mSegmentDurationChangeSlot);
			sequenceEditorView->unregisterSaveSlot(mSaveSlot);
		}
	}


	const Sequence& SequenceEditor::getSequence()
	{
		return mSequencePlayer->getSequence();
	}


	void SequenceEditor::segmentDurationChange(const SequenceTrack& track, const SequenceTrackSegment& segment, float amount)
	{
		// find the track
		for (auto& link : mSequencePlayer->mSequence->mSequenceTrackLinks)
		{
			auto trackPtr = link->mSequenceTrack.get();
			if ( trackPtr == &track)
			{
				for (auto _trackSegment : trackPtr->mSegments)
				{
					if (_trackSegment.get() == &segment)
					{
						_trackSegment->mDuration += amount;

						mSequencePlayer->updateDuration();

						break;
					}
				}

				break;
			}
			
		}
	}


	void SequenceEditor::save()
	{
		utility::ErrorState errorState;
		if (!errorState.check(mSequencePlayer->save(mSequencePlayer->mDefaultShow, errorState), "Error saving show!"))
		{
			nap::Logger::error(errorState.toString());
		}
	}

}
