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

		mController = std::make_unique<SequenceEditorController>(
			*mSequencePlayer.get(),
			mSequencePlayer->getSequence());

		return true;
	}


	void SequenceEditorController::segmentDurationChange(std::string segmentID, float amount)
	{
		// pause player thread

		// find the track
		for (auto& link : mSequence.mSequenceTrackLinks)
		{
			for (auto trackSegment : link->mSequenceTrack->mSegments)
			{
				if (trackSegment->mID == segmentID)
				{
					trackSegment->mDuration += amount;

					// update duration of sequence
					double longestTrack = 0.0;
					for (const auto& trackLink : mSequence.mSequenceTrackLinks)
					{
						double trackTime = 0.0;
						for (const auto& segment : trackLink->mSequenceTrack->mSegments)
						{
							double time = segment->mStartTime + segment->mDuration;
							if (time > trackTime)
							{
								trackTime = time;
							}
						}

						if (trackTime > longestTrack)
						{
							longestTrack = trackTime;
						}
					}

					mSequence.mDuration = longestTrack;

					break;
				}
			}
		}

		// resume player thread
	}


	const Sequence& SequenceEditor::getSequence() const
	{
		return mSequencePlayer->getSequence();
	}


	SequenceEditorController& SequenceEditor::getController()
	{
		return *mController.get();
	}


	void SequenceEditorController::save()
	{
		// pause player thread

		// save
		utility::ErrorState errorState;
		if (!errorState.check(mSequencePlayer.save(mSequencePlayer.mDefaultShow, errorState), "Error saving show!"))
		{
			nap::Logger::error(errorState.toString());
		}

		// resume player thread
	}


	void SequenceEditorController::insertSequence(double time)
	{
		// pause player thread

		//mSequencePlayer.

		// resume player thread
	}
}
