/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "entityptr.h"

RTTI_BEGIN_CLASS(nap::EntityPtr)
	RTTI_FUNCTION(nap::rtti::method::toString,			&nap::EntityPtr::toString)
	RTTI_FUNCTION(nap::rtti::method::assign,			&nap::EntityPtr::assign)
	RTTI_FUNCTION(nap::rtti::method::translateTargetID,	&nap::EntityPtr::translateTargetID)
RTTI_END_CLASS

namespace nap
{
	std::string EntityPtr::translateTargetID(const std::string& targetID)
	{
		size_t pos = targetID.find_last_of('/');
		if (pos == std::string::npos)
			return targetID;

		return targetID.substr(pos + 1);
	}
}
