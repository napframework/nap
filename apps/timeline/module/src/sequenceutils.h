#pragma once

// internal includes
#include "sequence.h"
#include "sequencetracksegmentnumeric.h"

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
		 * createDefaultSequence
		 * static method that creates a default sequence track
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence track
		 * @return a raw pointer to the newly created sequence track
		 */
		SequenceTrack* createSequenceTrackNumeric(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);
	}
}
