#pragma once

// internal includes
#include "sequence.h"
#include "sequencetracksegmentcurve.h"
#include "sequencetracksegmentevent.h"

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

		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		SequenceTrack* createSequenceEventTrack(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs
		);


		/**
		 * 
		 */
		template<typename T>
		SequenceTrack* createSequenceCurveTrack(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs)
		{
			int curveCount = 0;
			SequenceTrackTypes::Types trackType = SequenceTrackTypes::Types::UNKOWN;
			if (RTTI_OF(T) == RTTI_OF(glm::vec4))
			{
				curveCount = 4;
				trackType = SequenceTrackTypes::Types::VEC4;
			}
			else if (RTTI_OF(T) == RTTI_OF(glm::vec3))
			{
				curveCount = 3;
				trackType = SequenceTrackTypes::Types::VEC3;
			}
			else if (RTTI_OF(T) == RTTI_OF(glm::vec2))
			{
				curveCount = 2;
				trackType = SequenceTrackTypes::Types::VEC2;
			}
			else if (RTTI_OF(T) == RTTI_OF(float))
			{
				curveCount = 1;
				trackType = SequenceTrackTypes::Types::FLOAT;
			}

			//
			assert(trackType != SequenceTrackTypes::Types::UNKOWN);

			// create one segment
			std::unique_ptr<SequenceTrackSegmentCurve<T>> trackSegment = std::make_unique<SequenceTrackSegmentCurve<T>>();
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
			std::unique_ptr<SequenceTrackCurve<T>> sequenceTrack = std::make_unique<SequenceTrackCurve<T>>();
			sequenceTrack->mID = generateUniqueID(objectIDs);

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
