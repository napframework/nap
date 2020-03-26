// local includes
#include "sequenceeditor.h"
#include "sequencetrack.h"
#include "sequencetracksegment.h"
#include "sequenceutils.h"

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
		for (auto& track : mSequence.mTracks)
		{
			for (auto trackSegment : track->mSegments)
			{
				if (trackSegment->mID == segmentID)
				{
					trackSegment->mDuration += amount;

					// update duration of sequence
					double longestTrack = 0.0;
					for (const auto& track_ : mSequence.mTracks)
					{
						double trackTime = 0.0;
						for (const auto& segment : track_->mSegments)
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


	void SequenceEditorController::insertSequence(std::string trackID, double time)
	{
		// pause player thread

		for (auto& track : mSequence.mTracks)
		{
			if (track->mID == trackID)
			{
				int segmentCount = 1;
				for (auto& segment : track->mSegments)
				{
					if (segment->mStartTime < time && 
						segment->mStartTime + segment->mDuration > time)
					{
						std::unique_ptr<SequenceTrackSegment> newSegment = std::make_unique<SequenceTrackSegment>();
						newSegment->mStartTime = time;
						newSegment->mDuration = segment->mStartTime + segment->mDuration - time;
						segment->mDuration = newSegment->mStartTime - segment->mStartTime;

						newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.insert( track->mSegments.begin() + segmentCount, newSegmentResourcePtr);

						mCreatedObjects.emplace_back(std::move(newSegment));

						break;
					}

					segmentCount++;
				}
				
				break;
			}
		}

		// resume player thread
	}
}
