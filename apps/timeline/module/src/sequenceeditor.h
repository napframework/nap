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
			SegmentValueTypes valueType)
		{
			//
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			SequenceTrackSegment* segment = findSegment(trackID, segmentID);
			SequenceTrackSegmentNumeric& segmentNumeric = segment->getDerived<SequenceTrackSegmentNumeric>();

			if (segment != nullptr)
			{
				switch (valueType)
				{
				case BEGIN:
				{
					segmentNumeric.mStartValue += amount;
					segmentNumeric.mStartValue = math::clamp<float>(segmentNumeric.mStartValue, 0.0f, 1.0f);
				}
				break;
				case END:
				{
					segmentNumeric.mEndValue += amount;
					segmentNumeric.mEndValue = math::clamp<float>(segmentNumeric.mEndValue, 0.0f, 1.0f);
				}
				break;
				}

				updateSegments(lock);
			}
		}

		/**
		 * 
		 */
		void SequenceEditorController::insertCurvePointNumeric(
			const std::string& trackID,
			const std::string& segmentID,
			float pos)
		{
			//
			std::unique_lock<std::mutex> lock = mSequencePlayer.lock();

			// find segment
			SequenceTrackSegment* segment = findSegment(trackID, segmentID);

			if (segment != nullptr)
			{
				//
				auto& trackSegNum = segment->getDerived<SequenceTrackSegmentNumeric>();

				// iterate trough points of curve
				for (int i = 0; i < trackSegNum.mCurve->mPoints.size() - 1; i++)
				{
					// find the point the new point needs to get inserted after
					if (trackSegNum.mCurve->mPoints[i].mPos.mTime <= pos
						&& trackSegNum.mCurve->mPoints[i + 1].mPos.mTime > pos)
					{
						// create point
						math::FCurvePoint<float, float> p;
						p.mPos.mTime = pos;
						p.mPos.mValue = trackSegNum.mCurve->evaluate(pos);
						p.mInTan.mTime = -0.1f;
						p.mOutTan.mTime = 0.1f;
						p.mInTan.mValue = 0.0f;
						p.mOutTan.mValue = 0.0f;
						p.mTangentsAligned = true;
						p.mInterp = math::ECurveInterp::Bezier;

						// insert point
						trackSegNum.mCurve->mPoints.insert(trackSegNum.mCurve->mPoints.begin() + i + 1, p);
						trackSegNum.mCurve->invalidate();
						break;
					}
				}
			}
		}

		/**
		 * 
		 */
		void changeCurvePointNumeric(
			const std::string& trackID, 
			const std::string& segmentID, 
			const int index, 
			float time,
			float value)
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
					math::FCurvePoint<float, float>& curvePoint = trackSegFloat.mCurve->mPoints[index];
					curvePoint.mPos.mTime += time;
					curvePoint.mPos.mValue += value;
					curvePoint.mPos.mValue = math::clamp<float>(curvePoint.mPos.mValue, 0.0f, 1.0f);
					trackSegFloat.mCurve->invalidate();
				}
			}
		}

		/**
		 *
		 */
		void deleteCurvePoint(
			const std::string& trackID,
			const std::string& segmentID,
			const int index);

		/**
		 *
		 */
		void SequenceEditorController::changeTanPointNumeric(
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
				//
				auto& trackSegNumeric = segment->getDerived<SequenceTrackSegmentNumeric>();

				if (index < trackSegNumeric.mCurve->mPoints.size())
				{
					auto& curvePoint = trackSegNumeric.mCurve->mPoints[index];

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
					if (index == trackSegNumeric.mCurve->mPoints.size() - 1)
					{
						SequenceTrack* track = findTrack(trackID);
						if (track != nullptr)
						{
							for (int i = 0; i < track->mSegments.size(); i++)
							{
								if (track->mSegments[i].get() == segment &&
									i + 1 < track->mSegments.size())
								{
									auto& nextSegmentCurvePoint = track->mSegments[i + 1]->getDerived<SequenceTrackSegmentNumeric>().mCurve->mPoints[0];

									nextSegmentCurvePoint.mInTan.mTime = curvePoint.mInTan.mTime;
									nextSegmentCurvePoint.mInTan.mValue = curvePoint.mInTan.mValue;
									nextSegmentCurvePoint.mOutTan.mTime = curvePoint.mOutTan.mTime;
									nextSegmentCurvePoint.mOutTan.mValue = curvePoint.mOutTan.mValue;

								}
							}
						}
					}

					trackSegNumeric.mCurve->invalidate();
				}
			}
		}


		void assignNewParameterID(
			const std::string& trackID,
			const std::string& parameterID);


		template<typename T>
		void addNewTrackNumeric()
		{
			std::unique_lock<std::mutex> l = mSequencePlayer.lock();

			Sequence& sequence = mSequencePlayer.getSequence();

			SequenceTrack* newTrack = sequenceutils::createSequenceTrackNumeric(
				mSequencePlayer.mReadObjects,
				mSequencePlayer.mReadObjectIDs);
			sequence.mTracks.emplace_back(ResourcePtr<SequenceTrack>(newTrack));

			updateSegments(l);
		}


		void deleteTrack(const std::string& deleteTrackID);

		/**
		 * 
		 */
		SequencePlayer& getSequencePlayer() const;

		const Sequence& getSequence() const;
	protected:
		void updateSegments(const std::unique_lock<std::mutex>& lock);

		SequenceTrackSegment* findSegment(const std::string& trackID, const std::string& segmentID);
	
		SequenceTrack* findTrack(const std::string& trackID);

		void deleteObjectFromSequencePlayer(const std::string& id);
	protected:
		SequencePlayer&		mSequencePlayer;
	};
}
