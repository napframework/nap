/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "componentptr.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentPtrBase)
	RTTI_FUNCTION(nap::rtti::method::toString,				&nap::ComponentPtrBase::toString)
	RTTI_FUNCTION(nap::rtti::method::toObject,				&nap::ComponentPtrBase::toObject)
	RTTI_FUNCTION(nap::rtti::method::assign,				&nap::ComponentPtrBase::assign)
	RTTI_FUNCTION(nap::rtti::method::translateTargetID,		&nap::ComponentPtrBase::translateTargetID)
RTTI_END_CLASS

namespace nap
{
	std::string ComponentPtrBase::translateTargetID(const std::string& targetID)
	{
		size_t pos = targetID.find_last_of('/');
		return pos == std::string::npos ? targetID :
			targetID.substr(pos + 1);
	}
}
