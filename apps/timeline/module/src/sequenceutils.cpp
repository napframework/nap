// local includes
#include "sequenceutils.h"
#include "sequence.h"

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
		)
		{
			// create sequence track
			std::unique_ptr<SequenceTrackEvent> sequenceTrack = std::make_unique<SequenceTrackEvent>();
			sequenceTrack->mID = generateUniqueID(objectIDs);

			// assign return ptr
			SequenceTrack* returnPtr = sequenceTrack.get();

			// move ownership of unique ptrs
			createdObjects.emplace_back(std::move(sequenceTrack));

			return returnPtr;
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
				SequenceTrack* sequenceTrack
					= sequenceutils::createSequenceCurveTrack<float>(createdObjects, objectIDs);

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
	}
}
