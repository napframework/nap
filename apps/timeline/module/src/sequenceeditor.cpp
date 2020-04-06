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
			*mSequencePlayer.get());

		return true;
	}


	void SequenceEditorController::segmentDurationChange(const std::string& segmentID, float amount)
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


	void SequenceEditorController::insertSegment(const std::string& trackID, double time)
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
						std::unique_ptr<SequenceTrackSegment> newSegment = std::make_unique<SequenceTrackSegment>();
						newSegment->mStartTime = time;
						newSegment->mDuration = segment->mStartTime + segment->mDuration - time;

						// set the value by evaluation curve
						newSegment->mStartValue = segment->mCurve->evaluate((segment->mStartTime + segment->mDuration - time) / segment->mDuration);

						// check if there is a next segment
						if (segmentCount < track->mSegments.size())
						{
							// if there is a next segment, the new segments end value is the start value of the next segment ...
							newSegment->mEndValue = track->mSegments[segmentCount]->mStartValue;
						}
						else
						{
							// ... otherwise it just gets this segments end value
							newSegment->mEndValue = segment->mEndValue;
						}

						// the segment's end value gets the start value the newly inserted segment 
						segment->mEndValue = newSegment->mStartValue;

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

						//
						updateSegments(lock);

						break;
					}
					else if (segmentCount == track->mSegments.size())
					{
						// insert segment at the end of the list

						// create new segment & set parameters
						std::unique_ptr<SequenceTrackSegment> newSegment = std::make_unique<SequenceTrackSegment>();
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
					std::unique_ptr<SequenceTrackSegment> newSegment = std::make_unique<SequenceTrackSegment>();
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


		// resume player thread
	}


	void SequenceEditorController::changeSegmentValue(
		const std::string& trackID, 
		const std::string& segmentID, 
		float value,
		SegmentValueTypes type)
	{
		//
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		SequenceTrackSegment* segment = findSegment(trackID, segmentID);

		if (segment != nullptr)
		{
			switch (type)
			{
			case BEGIN:
			{
				segment->mStartValue += value;
				segment->mStartValue = math::clamp<float>(segment->mStartValue, 0.0f, 1.0f);
			}
				break;
			case END:
			{
				segment->mEndValue += value;
				segment->mEndValue = math::clamp<float>(segment->mEndValue, 0.0f, 1.0f);
			}
				break;
			}

			updateSegments(lock);
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
			ResourcePtr<SequenceTrackSegment> prevSeg = nullptr;
			for (auto trackSeg : track->mSegments)
			{
				if (prevSeg == nullptr)
				{
					// no previous segment, so bluntly assign the start value to the curve
					trackSeg->mCurve->mPoints[0].mPos.mValue = trackSeg->mStartValue;
				}
				else
				{
					// if we have a previous segment, the curve gets the value of the start value of the current segment
					trackSeg->mStartValue = prevSeg->mEndValue;
					prevSeg->mCurve->mPoints[prevSeg->mCurve->mPoints.size() - 1].mPos.mValue = trackSeg->mStartValue;
					trackSeg->mCurve->mPoints[0].mPos.mValue = trackSeg->mStartValue;
				}
				prevSeg = trackSeg;

				// if this is the last segment, bluntly assign the end value
				if (segmentCount == track->mSegments.size()-1)
				{
					trackSeg->mCurve->mPoints[trackSeg->mCurve->mPoints.size() - 1].mPos.mValue = trackSeg->mEndValue;
				}

				segmentCount++;
			}
		}
	}


	void SequenceEditorController::insertCurvePoint(const std::string& trackID, const std::string& segmentID, float pos)
	{
		//
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		// find segment
		SequenceTrackSegment* segment = findSegment(trackID, segmentID);

		if (segment != nullptr)
		{
			// iterate trough points of curve
			for (int i = 0; i < segment->mCurve->mPoints.size() - 1; i++)
			{
				// find the point the new point needs to get inserted after
				if (segment->mCurve->mPoints[i].mPos.mTime <= pos
					&& segment->mCurve->mPoints[i + 1].mPos.mTime > pos)
				{
					// create point
					math::FCurvePoint<float, float> p;
					p.mPos.mTime = pos;
					p.mPos.mValue = segment->mCurve->evaluate(pos);
					p.mInTan.mTime = -0.1f;
					p.mOutTan.mTime = 0.1f;
					p.mInTan.mValue = 0.0f;
					p.mOutTan.mValue = 0.0f;
					p.mTangentsAligned = true;
					p.mInterp = math::ECurveInterp::Bezier;

					// insert point
					segment->mCurve->mPoints.insert(segment->mCurve->mPoints.begin() + i + 1, p);
					segment->mCurve->invalidate();
					break;
				}
			}
		}
	}


	void SequenceEditorController::addNewTrack()
	{
		std::unique_lock<std::mutex> l = mSequencePlayer.lock();

		//
		Sequence& sequence = mSequencePlayer.getSequence();

		SequenceTrack* newTrack = sequenceutils::createDefaultSequenceTrack(mSequencePlayer.mReadObjects, mSequencePlayer.mReadObjectIDs);
		sequence.mTracks.emplace_back(ResourcePtr<SequenceTrack>(newTrack));

		updateSegments(l);
	}

	void SequenceEditorController::changeTanPoint(
		const std::string& trackID,
		const std::string& segmentID,
		const int index,
		TanPointTypes tanType,
		float time,
		float value)
	{
		//
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		// find segment
		SequenceTrackSegment* segment = findSegment(trackID, segmentID);

		if (segment != nullptr)
		{
			if (index < segment->mCurve->mPoints.size())
			{
				auto& curvePoint = segment->mCurve->mPoints[index];

				switch (tanType)
				{
				case IN:
				{
					if (curvePoint.mInTan.mTime + time < curvePoint.mOutTan.mTime)
					{
						curvePoint.mInTan.mTime += time;
						curvePoint.mInTan.mValue += value;

						if (curvePoint.mTangentsAligned)
						{
							curvePoint.mOutTan.mTime = -curvePoint.mInTan.mTime;
							curvePoint.mOutTan.mValue = -curvePoint.mInTan.mValue;
						}
					}
				}
					break;
				case OUT:
				{
					if (curvePoint.mOutTan.mTime + time > curvePoint.mInTan.mTime)
					{
						curvePoint.mOutTan.mTime += time;
						curvePoint.mOutTan.mValue += value;

						if (curvePoint.mTangentsAligned)
						{
							curvePoint.mInTan.mTime = -curvePoint.mOutTan.mTime;
							curvePoint.mInTan.mValue = -curvePoint.mOutTan.mValue;
						}
					}

				}
				break;
				}

				// is this the last control point ?
				// then also change the first control point of the next segment accordinly
				if (index == segment->mCurve->mPoints.size() - 1)
				{
					SequenceTrack* track = findTrack(trackID);
					if (track != nullptr)
					{
						for (int i = 0; i < track->mSegments.size(); i++)
						{
							if (track->mSegments[i].get() == segment &&
								i + 1 < track->mSegments.size())
							{
								auto& nextSegmentCurvePoint = track->mSegments[i + 1]->mCurve->mPoints[0];

								nextSegmentCurvePoint.mInTan.mTime = curvePoint.mInTan.mTime;
								nextSegmentCurvePoint.mInTan.mValue = curvePoint.mInTan.mValue;
								nextSegmentCurvePoint.mOutTan.mTime = curvePoint.mOutTan.mTime;
								nextSegmentCurvePoint.mOutTan.mValue = curvePoint.mOutTan.mValue;

							}
						}
					}
				}

				segment->mCurve->invalidate();
			}
		}
	}


	void SequenceEditorController::changeCurvePoint(
		const std::string& trackID, const std::string& segmentID, const int index, 
		float time,
		float value)
	{
		std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

		// find segment
		SequenceTrackSegment* segment = findSegment(trackID, segmentID);

		if (segment != nullptr)
		{
			if (index < segment->mCurve->mPoints.size())
			{
				//
				math::FCurvePoint<float, float>& curvePoint = segment->mCurve->mPoints[index];
				curvePoint.mPos.mTime += time;
				curvePoint.mPos.mValue += value;
				curvePoint.mPos.mValue = math::clamp<float>(curvePoint.mPos.mValue, 0.0f, 1.0f);
				segment->mCurve->invalidate();
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
			if (index < segment->mCurve->mPoints.size())
			{
				//
				segment->mCurve->mPoints.erase(segment->mCurve->mPoints.begin() + index);
				segment->mCurve->invalidate();
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

			track->mAssignedParameterID = parameterID;
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
}
