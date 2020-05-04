// local includes
#include "sequenceutils.h"
#include "sequence.h"
#include "sequencetrackevent.h"
#include "sequencetrackcurve.h"

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace sequenceutils
	{
		const std::string generateUniqueID(
			std::unordered_set<std::string>& objectIDs,
			const std::string& baseID)
		{
			std::string unique_id = baseID;

			int index = 1;
			while (objectIDs.find(unique_id) != objectIDs.end())
				unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

			objectIDs.insert(unique_id);

			return unique_id;
		}


		SequenceTrack* createSequenceEventTrack(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs
		){
			// create sequence track
			std::unique_ptr<SequenceTrackEvent> sequenceTrack = std::make_unique<SequenceTrackEvent>();
			sequenceTrack->mID = generateUniqueID(objectIDs);

			// assign return ptr
			SequenceTrack* returnPtr = sequenceTrack.get();

			// move ownership of unique ptrs
			createdObjects.emplace_back(std::move(sequenceTrack));

			return returnPtr;
		}


		Sequence* createSequence(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs)
		{
			// create the sequence
			std::unique_ptr<Sequence> sequence = std::make_unique<Sequence>();
			sequence->mID = generateUniqueID(objectIDs);
			sequence->mDuration = 1.0;

			// store pointer before moving ownership
			Sequence* returnSequence = sequence.get();

			// move ownership
			createdObjects.emplace_back(std::move(sequence));

			// finally return
			return returnSequence;
		}

		template<typename T>
		SequenceTrack* createSequenceCurveTrack(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs)
		{
			// create one segment
			std::unique_ptr<SequenceTrackSegmentCurve<T>> trackSegment = std::make_unique<SequenceTrackSegmentCurve<T>>();
			trackSegment->mID = generateUniqueID(objectIDs);
			trackSegment->mDuration = 1.0;
			trackSegment->mStartTime = 0.0;

			// create default curves
			trackSegment->mCurves.resize(trackSegment->getCurveCount());
			for (int i = 0; i < trackSegment->getCurveCount(); i++)
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

template nap::SequenceTrack* nap::sequenceutils::createSequenceCurveTrack<float>(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs);
template nap::SequenceTrack* nap::sequenceutils::createSequenceCurveTrack<glm::vec2>(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs);
template nap::SequenceTrack* nap::sequenceutils::createSequenceCurveTrack<glm::vec3>(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs);
template nap::SequenceTrack* nap::sequenceutils::createSequenceCurveTrack<glm::vec4>(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs);
