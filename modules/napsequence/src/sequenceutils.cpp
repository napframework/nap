/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceutils.h"
#include "sequence.h"
#include "sequencetrackevent.h"
#include "sequencetrackcurve.h"

#include <nap/logger.h>

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace sequenceutils
	{
		std::string generateUniqueID(std::unordered_set<std::string>& objectIDs, const std::string& baseID)
		{
			std::string unique_id = baseID;

			int index = 1;
			while (objectIDs.find(unique_id) != objectIDs.end())
				unique_id = utility::stringFormat("%s_%d", baseID.c_str(), ++index);

			objectIDs.insert(unique_id);

			return unique_id;
		}
	}
}
