/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "parameterstring.h"

RTTI_BEGIN_CLASS(nap::ParameterString)
	RTTI_PROPERTY("Value", &nap::ParameterString::mValue, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Size", &nap::ParameterString::mSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool ParameterString::init(utility::ErrorState & error)
	{
		if (!ParameterSimple<std::string>::init(error))
			return false;

		// Reserve size for string
		mValue.reserve(mSize);
		return true;
	}
}
