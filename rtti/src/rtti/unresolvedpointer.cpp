/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "unresolvedpointer.h"

namespace nap
{
	namespace rtti
	{
		std::string UnresolvedPointer::getResourceTargetID() const
		{
			ResolvedPath resolved_path;
			if (!mRTTIPath.resolve(mObject, resolved_path))
				return mTargetID;

			// If the type that we're processing has a function to translate the ID read from json into a different ID, we call it and use that ID.
			// This is used for pointers that have a different format in json.
			rttr::method translate_string_method = findMethodRecursive(resolved_path.getType(), method::translateTargetID);
			if (translate_string_method.is_valid())
			{
				rttr::variant translate_result = translate_string_method.invoke(rttr::instance(), mTargetID);
				return translate_result.to_string();
			}

			return mTargetID;
		}
	}
}
