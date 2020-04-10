// local includes
#include "sequenceutils.h"
#include "sequence.h"
#include "sequencetracksegmentnumeric.h"

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


		Sequence* createDefaultSequence(
			const std::vector<rtti::ObjectPtr<Parameter>>& parameters,
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs)
		{
			// create vector of sequence track links
			std::vector<ResourcePtr<SequenceTrack>> sequenceTracks;

			// now iterate trough all given parameters
			for (const auto& parameter : parameters)
			{
				// make a unique sequence track
				SequenceTrack* sequenceTrack = sequenceutils::createSequenceTrackNumeric(createdObjects, objectIDs);

				// make a resource pointer of the sequence track
				ResourcePtr<SequenceTrack> sequenceTrackResourcePtr = ResourcePtr<SequenceTrack>(sequenceTrack);

				sequenceTracks.emplace_back(sequenceTrackResourcePtr);
			}

			// create the sequence
			std::unique_ptr<Sequence> sequence = std::make_unique<Sequence>();
			sequence->mID = generateUniqueID(objectIDs);
			sequence->mDuration = 1.0;

			// assign the track links
			sequence->mTracks = sequenceTracks;

			// store pointer before moving ownership
			Sequence* returnSequence = sequence.get();

			// move ownership
			createdObjects.emplace_back(std::move(sequence));

			// finally return
			return returnSequence;
		}

		SequenceTrack* createSequenceTrackNumeric(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs)
		{
			// create one segment
			std::unique_ptr<SequenceTrackSegmentNumeric> trackSegment = std::make_unique<SequenceTrackSegmentNumeric>();
			trackSegment->mID = generateUniqueID(objectIDs);
			trackSegment->mDuration = 1.0;
			trackSegment->mStartTime = 0.0;

			// create default curve
			std::unique_ptr<math::FCurve<float, float>> segmentCurve = std::make_unique<math::FCurve<float, float>>();
			segmentCurve->mID = generateUniqueID(objectIDs);

			// assign curve
			trackSegment->mCurve = nap::ResourcePtr<math::FCurve<float, float>>(segmentCurve.get());

			// create sequence track
			std::unique_ptr<SequenceTrack> sequenceTrack = std::make_unique<SequenceTrack>();
			sequenceTrack->mID = generateUniqueID(objectIDs);
			sequenceTrack->mTrackType = SequenceTrackTypes::NUMERIC;

			// assign track segment
			sequenceTrack->mSegments.emplace_back(ResourcePtr<SequenceTrackSegment>(trackSegment.get()));

			// assign return ptr
			SequenceTrack* returnPtr = sequenceTrack.get();

			// move ownership of unique ptrs
			createdObjects.emplace_back(std::move(trackSegment));
			createdObjects.emplace_back(std::move(segmentCurve));
			createdObjects.emplace_back(std::move(sequenceTrack));

			return returnPtr;
		}
	}
}
