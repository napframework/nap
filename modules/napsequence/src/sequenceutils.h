/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequence.h"

namespace nap
{
	namespace sequenceutils
	{
		//////////////////////////////////////////////////////////////////////////

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
		NAPAPI Sequence* createEmptySequence(std::vector<std::unique_ptr<rtti::Object>>& createdObjects, std::unordered_set<std::string>& objectIDs);
	}
}
