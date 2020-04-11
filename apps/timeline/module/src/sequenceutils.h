#pragma once

// internal includes
#include "sequence.h"
#include "sequencetracksegmentnumeric.h"
#include "sequencetracksegmentvec.h"

namespace nap
{
	namespace sequenceutils
	{
		//////////////////////////////////////////////////////////////////////////

		const std::string generateUniqueID(
			std::unordered_set<std::string>& objectIDs,
			const std::string& baseID = "Generated");


		/**
		 * createDefaultSequence
		 * static method that creates a default sequence based on given parameters.
		 * It will created default sequence tracks for each given parameter
		 * @param parameters vector of parameters that we want to animate
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence
		 * @return a raw pointer to the newly created sequence
		 */
		Sequence* createDefaultSequence(
			const std::vector<rtti::ObjectPtr<Parameter>>& parameters,
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);

		 /**
		 * createSequenceTrackNumeric
		 * static method that creates a default sequence track
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence track
		 * @return a raw pointer to the newly created sequence track
		 */
		SequenceTrack* createSequenceTrackNumeric(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);

		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		/**
		 * 
		 */
		template<typename T>
		SequenceTrack* createSequenceTrackVector(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs)
		{
			int curveCount = 0;
			SequenceTrackTypes trackType = UNKOWN;
			if (RTTI_OF(T) == RTTI_OF(glm::vec4))
			{
				curveCount = 4;
				trackType = VEC4;
			}
			else if (RTTI_OF(T) == RTTI_OF(glm::vec3))
			{
				curveCount = 3;
				trackType = VEC3;
			}
			else if (RTTI_OF(T) == RTTI_OF(glm::vec2))
			{
				curveCount = 2;
				trackType = VEC2;
			}

			//
			assert(trackType != UNKOWN);

			// create one segment
			std::unique_ptr<SequenceTrackSegmentVec<T>> trackSegment = std::make_unique<SequenceTrackSegmentVec<T>>();
			trackSegment->mID = generateUniqueID(objectIDs);
			trackSegment->mDuration = 1.0;
			trackSegment->mStartTime = 0.0;

			// create default curves
			trackSegment->mCurves.resize(curveCount);
			for (int i = 0; i < curveCount; i++)
			{
				std::unique_ptr<math::FCurve<float, float>> segmentCurve = std::make_unique<math::FCurve<float, float>>();
				segmentCurve->mID = generateUniqueID(objectIDs);

				// assign curve
				trackSegment->mCurves[i] = nap::ResourcePtr<math::FCurve<float, float>>(segmentCurve.get());

				// move ownership
				createdObjects.emplace_back(std::move(segmentCurve));
			}

			// create sequence track
			std::unique_ptr<SequenceTrack> sequenceTrack = std::make_unique<SequenceTrack>();
			sequenceTrack->mID = generateUniqueID(objectIDs);
			sequenceTrack->mTrackType = trackType;

			// assign track segment
			sequenceTrack->mSegments.emplace_back(ResourcePtr<SequenceTrackSegment>(trackSegment.get()));

			// assign return ptr
			SequenceTrack* returnPtr = sequenceTrack.get();

			// move ownership of unique ptrs
			createdObjects.emplace_back(std::move(trackSegment));
			createdObjects.emplace_back(std::move(sequenceTrack));

			return returnPtr;
		}
	}
}
