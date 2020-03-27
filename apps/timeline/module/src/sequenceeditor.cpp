// local includes
#include "sequenceeditor.h"
#include "sequencetrack.h"
#include "sequencetracksegment.h"
#include "sequenceutils.h"

// external includes
#include <nap/logger.h>
#include <fcurve.h>

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
			ResourcePtr<SequenceTrackSegment> previousSegment = nullptr;
			for (auto trackSegment : track->mSegments)
			{
				if (trackSegment->mID == segmentID)
				{
					// check if new duration is valid
					bool valid = true;
					double newDuration = trackSegment->mDuration + amount;

					if (newDuration > 0.0)
					{
						if (previousSegment != nullptr)
						{
							if (trackSegment->mStartTime + newDuration < previousSegment->mStartTime + previousSegment->mDuration)
							{
								valid = false;
							}
						}
					}
					else
					{
						valid = false;
					}

					if (valid)
					{
						trackSegment->mDuration += amount;

						// update start times of all segments
						{
							ResourcePtr<SequenceTrackSegment> prevSeg = nullptr;
							for (auto trackSeg : track->mSegments)
							{
								if (prevSeg == nullptr)
								{
									trackSeg->mStartTime = 0.0;
								}
								else
								{
									trackSeg->mStartTime = prevSeg->mStartTime + prevSeg->mDuration;
								}
								prevSeg = trackSeg;
							}
						}

						// update duration of sequence
						double longestTrack = 0.0;
						for (const auto& otherTrack : mSequence.mTracks)
						{
							double trackTime = 0.0;
							for (const auto& segment : otherTrack->mSegments)
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
					}
					break;
				}

				previousSegment = trackSegment;
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

		// find the right track
		for (auto& track : mSequence.mTracks)
		{
			if (track->mID == trackID)
			{
				// track found

				// find the segment the new segment in inserted after
				int segmentCount = 1;
				for (auto& segment : track->mSegments)
				{
					if (segment->mStartTime < time && 
						segment->mStartTime + segment->mDuration > time)
					{
						// segment found

						// create new segment & set parameters
						std::unique_ptr<SequenceTrackSegment> newSegment = std::make_unique<SequenceTrackSegment>();
						newSegment->mStartTime = time;
						newSegment->mDuration = segment->mStartTime + segment->mDuration - time;

						// make new curve of segment
						std::unique_ptr<math::FCurve<float,float>> newCurve = std::make_unique<math::FCurve<float,float>>();
						newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
						newSegment->mCurve = ResourcePtr<math::FCurve<float, float>>(newCurve.get());
						
						// change duration of segment before inserted segment
						segment->mDuration = newSegment->mStartTime - segment->mStartTime;

						// generate unique id
						newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

						// wrap it in a resource ptr and insert it into the track
						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.insert( track->mSegments.begin() + segmentCount, newSegmentResourcePtr);

						// move ownership to sequence player
						mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));
						mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));

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
