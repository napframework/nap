// local includes
#include "sequence.h"

// external includes
#include <rtti/jsonreader.h>


RTTI_BEGIN_CLASS(nap::SequenceTrackLink)
	RTTI_PROPERTY("Parameter ID", &nap::SequenceTrackLink::mParameterID, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Sequence Track", &nap::SequenceTrackLink::mSequenceTrack, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::Sequence)
	RTTI_PROPERTY("Sequence Track Links", &nap::Sequence::mSequenceTrackLinks, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static const std::string generateUniqueID(
		std::unordered_set<std::string>& objectIDs,
		const std::string& baseID = "Generated")
	{
		std::string unique_id = baseID;

		int index = 1;
		while (objectIDs.find(unique_id) != objectIDs.end())
			unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

		objectIDs.insert(unique_id);

		return unique_id;
	}

	Sequence* Sequence::createDefaultSequence(
		const std::vector<rtti::ObjectPtr<ParameterFloat>>& parameters,
		std::vector<std::unique_ptr<rtti::Object>>& createdObjects)
	{
		// create vector of sequence track links
		std::vector<ResourcePtr<SequenceTrackLink>> sequenceTrackLinks;

		// used to keep track of generated ids
		std::unordered_set<std::string> object_ids;

		// now iterate trough all given parameters
		for (const auto& parameter : parameters)
		{
			// create a new unique sequence track link
			std::unique_ptr<SequenceTrackLink> sequenceTrackLink = std::make_unique<SequenceTrackLink>();
			sequenceTrackLink->mID = generateUniqueID(object_ids);
			object_ids.emplace(sequenceTrackLink->mID);

			// set the id the sequence track links to
			sequenceTrackLink->mParameterID = parameter->mID;

			// make a unique sequence track
			std::unique_ptr<SequenceTrack> sequenceTrack = std::make_unique<SequenceTrack>();
			sequenceTrack->mID = generateUniqueID(object_ids);
			object_ids.emplace(sequenceTrack->mID);
			
			// make a resource pointer of the sequence track
			ResourcePtr<SequenceTrack> sequenceTrackResourcePtr = ResourcePtr<SequenceTrack>(sequenceTrack.get());
			
			// assign it to the link
			sequenceTrackLink->mSequenceTrack = sequenceTrackResourcePtr;

			// now create a resource pointer to the track link
			ResourcePtr<SequenceTrackLink> sequenceTrackLinkResourcePtr(sequenceTrackLink.get());
			sequenceTrackLinks.emplace_back(sequenceTrackLinkResourcePtr);

			// move ownership of unique ptrs to created objects
			createdObjects.emplace_back(std::move(sequenceTrackLink));
			createdObjects.emplace_back(std::move(sequenceTrack));
		}

		// create the sequence
		std::unique_ptr<Sequence> sequence = std::make_unique<Sequence>();
		sequence->mID = generateUniqueID(object_ids);
		object_ids.emplace(sequence->mID);

		// assign the track links
		sequence->mSequenceTrackLinks = sequenceTrackLinks;
		
		// store pointer before moving ownership
		Sequence* returnSequence = sequence.get();

		// move ownership
		createdObjects.emplace_back(std::move(sequence));

		// finally return
		return returnSequence;
	}
}
