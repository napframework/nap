#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayeroutput.h"

namespace nap
{
	namespace sequenceutils
	{
		//////////////////////////////////////////////////////////////////////////

		using SequenceDefaultTrackFactoryFunc = std::unique_ptr<SequenceTrack>(*)(const SequencePlayerOutput*);

		/**
		 * generate a unique id
		 * @param objectIDs reference to collections of id's 
		 * @param baseID base id
		 * @return unique id
		 */
		NAPAPI const std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID = "Generated");


		/**
		 * creates a default sequence
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence
		 * @return a raw pointer to the newly created sequence, ownership of sequence is stored as a unique pointer in createdObjects
		 */
		NAPAPI Sequence* createDefaultSequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs, const std::vector<ResourcePtr<SequencePlayerOutput>>& outputs);


		/**
		 * can be used to register a default creation method to the factory. When createDefaultSequence is called, it will iterate trough the given outputs and
		 * create a SequenceTrack that fits the output. Whenever we create a new type of output, we should also add a way to create a default track
		 * @param type the type information of the sequence output
		 * @param method the factory method
		 * @return true on successful creation
		 */
		NAPAPI bool registerDefaultTrackCreator(rttr::type type, SequenceDefaultTrackFactoryFunc method);
	}
}
