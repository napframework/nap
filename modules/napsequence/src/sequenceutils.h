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

		/**
		 * generated a unique id
		 * @param objectIDs reference to collections of id's 
		 * @param baseID base id
		 */
		const std::string generateUniqueID(
			std::unordered_set<std::string>& objectIDs,
			const std::string& baseID = "Generated");


		/**
		 * createDefaultSequence
		 * creates a default sequence
		 * @param createdObject a reference to a vector that will be filled with unique pointers of created objects
		 * @param objectIDs a list of unique ids, used to created unique ids for each object in this sequence
		 * @return a raw pointer to the newly created sequence
		 */
		Sequence* createSequence(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);

		/**
		 * createSequenceEventTrack
		 * creates an sequence event track
		 * @param createdObjects reference to vector that contains unique pointers of the created objects
		 * @param objectIDs reference to a set of object ids, used to create unique id's for created objects
		 */
		SequenceTrack* createSequenceEventTrack(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);


		/**
		 * createSequenceCurveTrack
		 * creates an sequence curve track
		 * @param createdObjects reference to vector that contains unique pointers of the created objects
		 * @param objectIDs reference to a set of object ids, used to create unique id's for created objects
		 */
		template<typename T>
		SequenceTrack* createSequenceCurveTrack(
			std::vector<std::unique_ptr<rtti::Object>>& createdObjects,
			std::unordered_set<std::string>& objectIDs);
	}
}
