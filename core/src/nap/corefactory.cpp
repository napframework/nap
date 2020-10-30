/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "corefactory.h"
#include "core.h"

namespace nap
{
	CoreFactory::CoreFactory(Core& core) :
		mCore(&core)
	{
	}

	rtti::Object* CoreFactory::createDefaultObject(rtti::TypeInfo typeInfo)
	{
		// Find a constructor taking Core& as argument. If it's there, use it, otherwise default to the base implementation
		rttr::constructor constructor = typeInfo.get_constructor({ RTTI_OF(Core) });
		if (constructor.is_valid())
			return constructor.invoke(*mCore).get_value<rtti::Object*>();

		return rtti::Factory::createDefaultObject(typeInfo);
	}
}