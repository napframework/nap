#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayer.h"
#include "sequenceutils.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceEditorController;

	/**
	 * SequenceEditor
	 * The SequenceEditor is responsible for editing the sequence (model) and makes sure the model stays valid during editing.
	 * It also holds a resource ptr to a player, to make sure that editing the sequence stays thread safe.
	 */
	class NAPAPI SequenceEditor : 
		public Resource
	{
		friend class SequenceEditorGUI;

		RTTI_ENABLE(Resource)
	public:
		virtual bool init(utility::ErrorState& errorState);
	public:
		// properties
		ResourcePtr<SequencePlayer> mSequencePlayer = nullptr; ///< Property: 'Sequence Player' ResourcePtr to the sequence player

		const Sequence& getSequence() const;
	protected:
		std::unique_ptr<SequenceEditorController> mController = nullptr;
	private:
		SequenceEditorController& getController();
	};

	/**
	 * 
	 */
	enum TanPointTypes
	{
		IN,
		OUT
	};

	/**
	 * 
	 */
	enum SegmentValueTypes
	{
		BEGIN,
		END
	};

	/**
	 * SequenceEditorController 
	 * The actual controller with methods that a view can call
	 */
	class SequenceEditorController
	{
	public:
		/**
		 * Constructor
		 * @param sequencePlayer reference to the sequence player
		 * @param sequence reference to the sequence ( model )
		 */
		SequenceEditorController(SequencePlayer& sequencePlayer) 
			: mSequencePlayer(sequencePlayer) {}

		/**
		 * segmentDurationChange
		 * @param segmentID the id of the segement we need to edit
		 * @param amount the amount that the duration of this segment should change
		 */
		void segmentDurationChange(
			const std::string& segmentID, 
			float amount);

		/**
		 * save
		 * saves the sequence
		 */
		void save();

		/**
		 * insertSegment
		 * insert segment at given time
		 * @param trackID the track that the segment gets inserted to
		 * @param time the time at which the track gets inserted
		 */
		void insertSegmentNumeric(
			const std::string& trackID, 
			double time);

		/**
		 * deleteSegment
		 * delete segment 
		 * @param trackID the track in which the segment gets deleted 
		 * @param segmentID the segment ID that needs to be deleted
		 */
		void deleteSegment(
			const std::string& trackID,
			const std::string& segmentID);

		/**
		 * changeSegmentValue
		 * changes the end value of this segment and updates the track accordingly
		 * @param trackID the track in which the segment gets updated
		 * @param segmentID the segment ID that needs to be updated
		 * @param amount the amount that the end value needs to change
		 * @param type the type of value that needs to change ( first or last value )
		 */
		void changeSegmentValueNumeric(
			const std::string& trackID,
			const std::string& segmentID,
			float amount,
			SegmentValueTypes valueType);

		/**
		 * 
		 */
		void insertCurvePointNum(
			const std::string& trackID,
			const std::string& segmentID,
			float pos);
		
		/**
		 * 
		 */
		void changeCurvePointNumeric(
			const std::string& trackID,
			const std::string& segmentID,
			const int index,
			float time,
			float value);

		/**
		 *
		 */
		void deleteCurvePointNum(
			const std::string& trackID,
			const std::string& segmentID,
			const int index);

		/**
		 *
		 */
		void changeTanPointNum(
			const std::string& trackID,
			const std::string& segmentID,
			const int index,
			TanPointTypes tanType,
			float time,
			float value);

		/**
		 * 
		 */
		void assignNewParameterID(
			const std::string& trackID,
			const std::string& parameterID);

		/**
		 * 
		 */
		void addNewTrackNumeric();


		/**
		 * 
		 */
		void deleteTrack(const std::string& deleteTrackID);

		/**
		 * 
		 */
		SequencePlayer& getSequencePlayer() const;

		const Sequence& getSequence() const;

		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		/**
		 * 
		 */
		template<typename T>
		void addNewTrackVector()
		{
			std::unique_lock<std::mutex> l = mSequencePlayer.lock();

			Sequence& sequence = mSequencePlayer.getSequence();

			SequenceTrack* newTrack = sequenceutils::createSequenceTrackVector<T>(
				mSequencePlayer.mReadObjects,
				mSequencePlayer.mReadObjectIDs);
			sequence.mTracks.emplace_back(ResourcePtr<SequenceTrack>(newTrack));

			updateSegmentsVec<T>(*newTrack);
		}

		/**
		 * 
		 */
		template<typename T>
		void insertSegmentVec(
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
							std::unique_ptr<SequenceTrackSegmentVec<T>> newSegment = std::make_unique<SequenceTrackSegmentVec<T>>();
							newSegment->mStartTime = time;
							newSegment->mDuration = segment->mStartTime + segment->mDuration - time;

							//
							SequenceTrackSegmentVec<T>& segmentVec = segment->getDerived<SequenceTrackSegmentVec<T>>();

							// set the value by evaluation curve
							for (int v = 0; v < segmentVec.mCurves.size(); v++)
							{
								newSegment->mStartValue[v] = segmentVec.mCurves[v]->evaluate((segment->mStartTime + segment->mDuration - time) / segment->mDuration);
							}

							// check if there is a next segment
							if (segmentCount < track->mSegments.size())
							{
								// if there is a next segment, the new segments end value is the start value of the next segment ...
								SequenceTrackSegmentVec<T>& nextSegmentFloat = track->mSegments[segmentCount]->getDerived<SequenceTrackSegmentVec<T>&>();
								newSegment->mEndValue = nextSegmentFloat.mStartValue;
							}
							else
							{
								// ... otherwise it just gets this segments end value
								newSegment->mEndValue = segmentVec.mEndValue;
							}

							// the segment's end value gets the start value the newly inserted segment 
							segmentVec.mEndValue = newSegment->mStartValue;

							// make new curve of segment
							int curveCount = 0;
							if (RTTI_OF(T) == RTTI_OF(glm::vec3))
							{
								curveCount = 3;
							}
							else if (RTTI_OF(T) == RTTI_OF(glm::vec2))
							{
								curveCount = 2;
							}
							else if (RTTI_OF(T) == RTTI_OF(glm::vec4))
							{
								curveCount = 4;
							}

							newSegment->mCurves.resize(curveCount);
							for (int v = 0; v < curveCount; v++)
							{
								std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
								newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
								newSegment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(newCurve.get());
								
								// move ownership to sequence player
								mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));
							}

							// change duration of segment before inserted segment
							segment->mDuration = newSegment->mStartTime - segment->mStartTime;

							// generate unique id
							newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

							// wrap it in a resource ptr and insert it into the track
							ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
							track->mSegments.insert(track->mSegments.begin() + segmentCount, newSegmentResourcePtr);

							// move ownership to sequence player
							mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));

							//
							updateSegmentsVec<T>(*(track.get()));

							break;
						}
						else if (segmentCount == track->mSegments.size())
						{
							// insert segment at the end of the list

							// create new segment & set parameters
							std::unique_ptr<SequenceTrackSegmentVec<T>> newSegment =
								std::make_unique<SequenceTrackSegmentVec<T>>();
							newSegment->mStartTime = segment->mStartTime + segment->mDuration;
							newSegment->mDuration = time - newSegment->mStartTime;

							// make new curve of segment
							int curveCount = 0;
							if (RTTI_OF(T) == RTTI_OF(glm::vec3))
							{
								curveCount = 3;
							}
							else if (RTTI_OF(T) == RTTI_OF(glm::vec2))
							{
								curveCount = 2;
							}
							else if (RTTI_OF(T) == RTTI_OF(glm::vec4))
							{
								curveCount = 4;
							}

							newSegment->mCurves.resize(curveCount);
							for (int v = 0; v < curveCount; v++)
							{
								std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
								newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
								newSegment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(newCurve.get());
								mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));
							}

							// generate unique id
							newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

							// wrap it in a resource ptr and insert it into the track
							ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
							track->mSegments.emplace_back(newSegmentResourcePtr);

							// move ownership to sequence player
							mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));

							//
							updateSegmentsVec<T>(*(track.get()));

							break;
						}


						segmentCount++;
					}

					//
					if (track->mSegments.size() == 0)
					{
						// create new segment & set parameters
						std::unique_ptr<SequenceTrackSegmentVec<T>> newSegment = std::make_unique<SequenceTrackSegmentVec<T>>();
						newSegment->mStartTime = 0.0;
						newSegment->mDuration = time - newSegment->mStartTime;

						// make new curve of segment
						int curveCount = 0;
						if (RTTI_OF(T) == RTTI_OF(glm::vec3))
						{
							curveCount = 3;
						}
						else if (RTTI_OF(T) == RTTI_OF(glm::vec2))
						{
							curveCount = 2;
						}
						else if (RTTI_OF(T) == RTTI_OF(glm::vec4))
						{
							curveCount = 4;
						}

						// make new curve of segment
						newSegment->mCurves.resize(curveCount);
						for (int v = 0; v < curveCount; v++)
						{
							std::unique_ptr<math::FCurve<float, float>> newCurve = std::make_unique<math::FCurve<float, float>>();
							newCurve->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);
							newSegment->mCurves[v] = ResourcePtr<math::FCurve<float, float>>(newCurve.get());
							mSequencePlayer.mReadObjects.emplace_back(std::move(newCurve));
						}

						// generate unique id
						newSegment->mID = sequenceutils::generateUniqueID(mSequencePlayer.mReadObjectIDs);

						// wrap it in a resource ptr and insert it into the track
						ResourcePtr<SequenceTrackSegment> newSegmentResourcePtr(newSegment.get());
						track->mSegments.emplace_back(newSegmentResourcePtr);

						// move ownership to sequence player
						mSequencePlayer.mReadObjects.emplace_back(std::move(newSegment));

						//
						updateSegmentsVec<T>(*(track.get()));
					}
					break;
				}
			}
		}

		/**
		 * 
		 */
		template<typename T>
		void changeSegmentValueVec(
			const std::string& trackID,
			const std::string& segmentID,
			float amount,
			int curveIndex,
			SegmentValueTypes valueType)
		{
			//
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			SequenceTrack* track = findTrack(trackID);
			assert(track != nullptr);

			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr);

			SequenceTrackSegmentVec<T>& segmentVec = segment->getDerived<SequenceTrackSegmentVec<T>>();
			assert(curveIndex < segmentVec.mCurves.size());

			if (segment != nullptr)
			{
				switch (valueType)
				{
				case BEGIN:
				{
					segmentVec.mStartValue[curveIndex] += amount;
					segmentVec.mStartValue[curveIndex] = math::clamp<float>(segmentVec.mStartValue[curveIndex], 0.0f, 1.0f);
				}
				break;
				case END:
				{
					segmentVec.mEndValue[curveIndex] += amount;
					segmentVec.mEndValue[curveIndex] = math::clamp<float>(segmentVec.mEndValue[curveIndex], 0.0f, 1.0f);
				}
				break;
				}
			}

			//
			updateSegmentsVec<T>(*track);
		}

		/**
		 * 
		 */
		template<typename T>
		void updateSegmentsVec(SequenceTrack& track)
		{
			// update start time and duration of all segments
			ResourcePtr<SequenceTrackSegmentVec<T>> prevSeg = nullptr;
			for (auto trackSeg : track.mSegments)
			{
				auto& trackSegVec = trackSeg->getDerived<SequenceTrackSegmentVec<T>>();

				if (prevSeg == nullptr)
				{
					// no previous segment, so bluntly assign the start value to the curve
					for (int v = 0; v < trackSegVec.mCurves.size(); v++)
					{
						trackSegVec.mCurves[v]->mPoints[0].mPos.mValue = trackSegVec.mStartValue[v];
					}
				}
				else
				{
					// if we have a previous segment, the curve gets the value of the start value of the current segment
					trackSegVec.mStartValue = prevSeg->mEndValue;
					for (int v = 0; v < trackSegVec.mCurves.size(); v++)
					{
						prevSeg->mCurves[v]->mPoints[prevSeg->mCurves[v]->mPoints.size() - 1].mPos.mValue = trackSegVec.mStartValue[v];
						trackSegVec.mCurves[v]->mPoints[0].mPos.mValue = trackSegVec.mStartValue[v];
					}
				}
				prevSeg = &trackSegVec;

				// if this is the last segment, bluntly assign the end value
				int lastSegmentIndex = track.mSegments.size() - 1;
				if (track.mSegments[lastSegmentIndex]->mID == trackSegVec.mID)
				{
					for (int v = 0; v < trackSegVec.mCurves.size(); v++)
					{
						trackSegVec.mCurves[v]->mPoints[trackSegVec.mCurves[v]->mPoints.size() - 1].mPos.mValue = trackSegVec.mEndValue[v];
					}
				}
			}
		}

		/**
		 * 
		 */
		template<typename T>
		void insertCurvePointVec(
			const std::string& trackID,
			const std::string& segmentID,
			float pos,
			int curveIndex)
		{
			//
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr);

			//
			auto& trackSegNum = segment->getDerived<SequenceTrackSegmentVec<T>>();
			assert(curveIndex < trackSegNum.mCurves.size());

			// iterate trough points of curve
			for (int i = 0; i < trackSegNum.mCurves[curveIndex]->mPoints.size() - 1; i++)
			{
				// find the point the new point needs to get inserted after
				if (trackSegNum.mCurves[curveIndex]->mPoints[i].mPos.mTime <= pos
					&& trackSegNum.mCurves[curveIndex]->mPoints[i + 1].mPos.mTime > pos)
				{
					// create point
					math::FCurvePoint<float, float> p;
					p.mPos.mTime = pos;
					p.mPos.mValue = trackSegNum.mCurves[curveIndex]->evaluate(pos);
					p.mInTan.mTime = -0.1f;
					p.mOutTan.mTime = 0.1f;
					p.mInTan.mValue = 0.0f;
					p.mOutTan.mValue = 0.0f;
					p.mTangentsAligned = true;
					p.mInterp = math::ECurveInterp::Bezier;

					// insert point
					trackSegNum.mCurves[curveIndex]->mPoints.insert(trackSegNum.mCurves[curveIndex]->mPoints.begin() + i + 1, p);
					trackSegNum.mCurves[curveIndex]->invalidate();
					break;
				}
			}
			
		}

		template<typename T>
		void deleteCurvePointVec(
			const std::string& trackID,
			const std::string& segmentID,
			const int index,
			int curveIndex)
		{
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr);

			//
			auto& trackSegVec = segment->getDerived<SequenceTrackSegmentVec<T>>();
			assert(curveIndex < trackSegVec.mCurves.size());

			if (index < trackSegVec.mCurves[curveIndex]->mPoints.size())
			{
				//
				trackSegVec.mCurves[curveIndex]->mPoints.erase(trackSegVec.mCurves[curveIndex]->mPoints.begin() + index);
				trackSegVec.mCurves[curveIndex]->invalidate();
			}
		}

		template<typename T>
		void changeCurvePointVec(
			const std::string& trackID,
			const std::string& segmentID,
			const int pointIndex,
			const int curveIndex,
			float time,
			float value)
		{
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr);

			//
			auto& trackSegVec = segment->getDerived<SequenceTrackSegmentVec<T>>();
			assert(curveIndex < trackSegVec.mCurves.size());
			assert(pointIndex < trackSegVec.mCurves[curveIndex]->mPoints.size());

			//
			math::FCurvePoint<float, float>& curvePoint 
				= trackSegVec.mCurves[curveIndex]->mPoints[pointIndex];
			curvePoint.mPos.mTime += time;
			curvePoint.mPos.mValue += value;
			curvePoint.mPos.mValue = math::clamp<float>(curvePoint.mPos.mValue, 0.0f, 1.0f);
			trackSegVec.mCurves[curveIndex]->invalidate();
		}

		template<typename T>
		void changeTanPointVec(
			const std::string& trackID,
			const std::string& segmentID,
			const int pointIndex,
			const int curveIndex,
			TanPointTypes tanType,
			float time,
			float value)
		{
			//
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			assert(segment != nullptr);
			
			//
			auto& trackSegVec = segment->getDerived<SequenceTrackSegmentVec<T>>();
			assert(curveIndex < trackSegVec.mCurves.size());
			assert(pointIndex < trackSegVec.mCurves[curveIndex]->mPoints.size());

			//
			auto& curvePoint = trackSegVec.mCurves[curveIndex]->mPoints[pointIndex];
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
			if (pointIndex == trackSegVec.mCurves[curveIndex]->mPoints.size() - 1)
			{
				SequenceTrack* track = findTrack(trackID);
				assert(track != nullptr);

				for (int i = 0; i < track->mSegments.size(); i++)
				{
					if (track->mSegments[i].get() == segment &&
						i + 1 < track->mSegments.size())
					{
						auto& nextSegmentCurvePoint =
							track->mSegments[i + 1]->getDerived<SequenceTrackSegmentVec<T>>().mCurves[curveIndex]->mPoints[0];

						nextSegmentCurvePoint.mInTan.mTime = curvePoint.mInTan.mTime;
						nextSegmentCurvePoint.mInTan.mValue = curvePoint.mInTan.mValue;
						nextSegmentCurvePoint.mOutTan.mTime = curvePoint.mOutTan.mTime;
						nextSegmentCurvePoint.mOutTan.mValue = curvePoint.mOutTan.mValue;

					}
				}
			}

			trackSegVec.mCurves[curveIndex]->invalidate();
		}
	protected:
		void updateSegments();

		SequenceTrackSegment* findSegment(const std::string& trackID, const std::string& segmentID);
	
		SequenceTrack* findTrack(const std::string& trackID);

		void deleteObjectFromSequencePlayer(const std::string& id);
	protected:
		SequencePlayer&		mSequencePlayer;
	};
}
