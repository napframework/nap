/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// internal includes
#include "sequence.h"
#include "sequenceplayeroutput.h"

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
		NAPAPI std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID = "Generated");
	}
}
