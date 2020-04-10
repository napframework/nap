// local includes
#include "sequenceeditor.h"
#include "sequencetrack.h"
#include "sequencetracksegment.h"
#include "sequenceutils.h"
#include "sequencetracksegmentnumeric.h"

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
			*mSequencePlayer.get());

		return true;
	}


	void SequenceEditorController::segmentDurationChange(
		const std::string& segmentID,
		float amount)
	{
		// pause player thread
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		//
		Sequence& sequence = mSequencePlayer.getSequence();

		// find the track
		for (auto& track : sequence.mTracks)
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

						updateSegments(lock);
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
		// save
		utility::ErrorState errorState;
		if (!errorState.check(mSequencePlayer.save(mSequencePlayer.mDefaultShow, errorState), "Error saving show!"))
		{
			nap::Logger::error(errorState.toString());
		}
	}


	void SequenceEditorController::insertSegmentNumeric(
		const std::string& trackID,
		double time)
	{
		// pause player thread
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		//
		Sequence& sequence = mSequencePlayer.getSequence();

		// find the right track
		for (auto& track : sequence.mTracks)
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
						std::unique_ptr<SequenceTrackSegmentNumeric> newSegment = std::make_unique<SequenceTrackSegmentNumeric>();
						newSegment->mStartTime = time;
						newSegment->mDuration = segment->mStartTime + segment->mDuration - time;

						//
						SequenceTrackSegmentNumeric& segmentFloat = segment->getDerived<SequenceTrackSegmentNumeric>();

						// set the value by evaluation curve
						newSegment->mStartValue = segmentFloat.mCurve->evaluate((segment->mStartTime + segment->mDuration - time) / segment->mDuration);

						// check if there is a next segment
						if (segmentCount < track->mSegments.size())
						{
							// if there is a next segment, the new segments end value is the start value of the next segment ...
							SequenceTrackSegmentNumeric& nextSegmentFloat = track->mSegments[segmentCount]->getDerived<SequenceTrackSegmentNumeric>();
							newSegment->mEndValue = nextSegmentFloat.mStartValue;
						}
						else
						{
							// ... otherwise it just gets this segments end value
							newSegment->mEndValue = segmentFloat.mEndValue;
						}

						// the segment's end value gets the start value the newly inserted segment 
						segmentFloat.mEndValue = newSegment->mStartValue;

						// make new curve of segment
						std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
						newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
						newSegment->mCurve = ResourcePtr<math::FCurve<float, float>>(newCurve.get());

						// change duration of segment before inserted segment
						segment->mDuration = newSegment->mStartTime - segment->mStartTime;

						// generate unique id
						newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

						// wrap it in a resource ptr and insert it into the track
						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.insert(track->mSegments.begin() + segmentCount, newSegmentResourcePtr);

						// move ownership to sequence player
						mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));
						mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));

						//
						updateSegments(lock);

						break;
					}
					else if (segmentCount == track->mSegments.size())
					{
						// insert segment at the end of the list

						// create new segment & set parameters
						std::unique_ptr<SequenceTrackSegmentNumeric> newSegment = std::make_unique<SequenceTrackSegmentNumeric>();
						newSegment->mStartTime = segment->mStartTime + segment->mDuration;
						newSegment->mDuration = time - newSegment->mStartTime;

						// make new curve of segment
						std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
						newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
						newSegment->mCurve = ResourcePtr<math::FCurve<float, float>>(newCurve.get());

						// generate unique id
						newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

						// wrap it in a resource ptr and insert it into the track
						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.emplace_back(newSegmentResourcePtr);

						// move ownership to sequence player
						mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));
						mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));

						//
						updateSegments(lock);

						break;
					}


					segmentCount++;
				}

				//
				if (track->mSegments.size() == 0)
				{
					// create new segment & set parameters
					std::unique_ptr<SequenceTrackSegmentNumeric> newSegment = std::make_unique<SequenceTrackSegmentNumeric>();
					newSegment->mStartTime = 0.0;
					newSegment->mDuration = time - newSegment->mStartTime;

					// make new curve of segment
					std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
					newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
					newSegment->mCurve = ResourcePtr<math::FCurve<float, float>>(newCurve.get());

					// generate unique id
					newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

					// wrap it in a resource ptr and insert it into the track
					ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
					track->mSegments.emplace_back(newSegmentResourcePtr);

					// move ownership to sequence player
					mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));
					mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));

					//
					updateSegments(lock);
				}
				break;
			}
		}
	}

	void SequenceEditorController::deleteSegment(const std::string& trackID, const std::string& segmentID)
	{
		// pause player thread
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		//
		Sequence& sequence = mSequencePlayer.getSequence();

		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				int segmentIndex = 0;
				for (auto& segment : track->mSegments)
				{
					if (segment->mID == segmentID)
					{
						// store the duration of the segment that we are deleting
						double duration = segment->mDuration;

						// erase it from the list
						track->mSegments.erase(track->mSegments.begin() + segmentIndex);
						
						// get the segment that is now at the previous deleted segments position
						if (track->mSegments.begin() + segmentIndex != track->mSegments.end())
						{
							// add the duration
							track->mSegments[segmentIndex]->mDuration += duration;
						}

						deleteObjectFromSequencePlayer(segmentID);

						// update segments
						updateSegments(lock);

						break;
					}
					segmentIndex++;
				}

				break;
			}
		}
	}


	void SequenceEditorController::deleteCurvePoint(
		const std::string& trackID,
		const std::string& segmentID,
		const int index)
	{
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		// find segment
		SequenceTrackSegment* segment = findSegment(trackID, segmentID);

		if (segment != nullptr)
		{
			//
			auto& trackSegFloat = segment->getDerived<SequenceTrackSegmentNumeric>();

			if (index < trackSegFloat.mCurve->mPoints.size())
			{
				//
				trackSegFloat.mCurve->mPoints.erase(trackSegFloat.mCurve->mPoints.begin() + index);
				trackSegFloat.mCurve->invalidate();
			}
		}
	}


	SequenceTrackSegment* SequenceEditorController::findSegment(const std::string& trackID, const std::string& segmentID)
	{
		//
		Sequence& sequence = mSequencePlayer.getSequence();

		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				for (auto& segment : track->mSegments)
				{
					if (segment->mID == segmentID)
					{
						return segment.get();
					}
				}
			}
		}

		return nullptr;
	}


	SequenceTrack* SequenceEditorController::findTrack(const std::string& trackID)
	{
		//
		Sequence& sequence = mSequencePlayer.getSequence();

		for (auto& track : sequence.mTracks)
		{
			if (track->mID == trackID)
			{
				return track.get();
			}
		}

		return nullptr;
	}


	void SequenceEditorController::assignNewParameterID(
		const std::string& trackID,
		const std::string& parameterID)
	{
		SequenceTrack* track = findTrack(trackID);

		if (track != nullptr)
		{
			std::unique_lock<std::mutex> l = mSequencePlayer.lock();

			if (mSequencePlayer.createProcessor(parameterID, trackID))
			{
				track->mAssignedParameterID = parameterID;
			}
		}
	}


	void SequenceEditorController::deleteTrack(const std::string& deleteTrackID)
	{
		//
		Sequence& sequence = mSequencePlayer.getSequence();

		int index = 0;
		for (const auto& track : sequence.mTracks)
		{
			if (track->mID == deleteTrackID)
			{
				sequence.mTracks.erase(sequence.mTracks.begin() + index);

				deleteObjectFromSequencePlayer(deleteTrackID);

				break;
			}
			index++;
		}
	}


	SequencePlayer& SequenceEditorController::getSequencePlayer() const
	{
		return mSequencePlayer;
	}


	const Sequence& SequenceEditorController::getSequence() const
	{
		return *mSequencePlayer.mSequence;
	}


	void SequenceEditorController::deleteObjectFromSequencePlayer(const std::string& id)
	{
		if (mSequencePlayer.mReadObjectIDs.find(id) != mSequencePlayer.mReadObjectIDs.end())
		{
			mSequencePlayer.mReadObjectIDs.erase(id);
		}

		for (int i = 0; i < mSequencePlayer.mReadObjects.size(); i++)
		{
			if (mSequencePlayer.mReadObjects[i]->mID == id)
			{
				mSequencePlayer.mReadObjects.erase(mSequencePlayer.mReadObjects.begin() + i);
				break;
			}
		}
	}


	void SequenceEditorController::updateSegments(const std::unique_lock<std::mutex>& lock)
	{
		//
		Sequence& sequence = mSequencePlayer.getSequence();

		for (auto& track : sequence.mTracks)
		{
			// update start time and duration of all segments
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
					prevSeg->mDuration = trackSeg->mStartTime - prevSeg->mStartTime;
				}
				prevSeg = trackSeg;
			}
		}

		// update duration of sequence
		double longestTrack = 0.0;
		for (const auto& otherTrack : sequence.mTracks)
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

		sequence.mDuration = longestTrack;

		// make sure start & end value align
		for (auto& track : sequence.mTracks)
		{
			//
			int segmentCount = 0;

			// update start time and duration of all segments
			ResourcePtr<SequenceTrackSegmentNumeric> prevSeg = nullptr;
			for (auto trackSeg : track->mSegments)
			{
				if (trackSeg->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentNumeric)))
				{
					auto& trackSegFloat = trackSeg->getDerived<SequenceTrackSegmentNumeric>();

					if (prevSeg == nullptr)
					{
						// no previous segment, so bluntly assign the start value to the curve
						trackSegFloat.mCurve->mPoints[0].mPos.mValue = trackSegFloat.mStartValue;
					}
					else
					{
						// if we have a previous segment, the curve gets the value of the start value of the current segment
						trackSegFloat.mStartValue = prevSeg->mEndValue;
						prevSeg->mCurve->mPoints[prevSeg->mCurve->mPoints.size() - 1].mPos.mValue = trackSegFloat.mStartValue;
						trackSegFloat.mCurve->mPoints[0].mPos.mValue = trackSegFloat.mStartValue;
					}
					prevSeg = &trackSegFloat;

					// if this is the last segment, bluntly assign the end value
					if (segmentCount == track->mSegments.size() - 1)
					{
						trackSegFloat.mCurve->mPoints[trackSegFloat.mCurve->mPoints.size() - 1].mPos.mValue = trackSegFloat.mEndValue;
					}
				}

				segmentCount++;
			}
		}
	}
}
