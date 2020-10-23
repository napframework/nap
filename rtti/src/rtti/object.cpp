/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "object.h"
#include "objectptr.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::rtti::Object)
	RTTI_PROPERTY(nap::rtti::sIDPropertyName, &nap::rtti::Object::mID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace rtti
	{
		bool Object::isIDProperty(rtti::Instance& object, const rtti::Property& property)
		{
			return object.get_derived_type().is_derived_from<Object>() && std::string(property.get_name().data()) == sIDPropertyName;
		}


		// Note: Even though the RTTIObject constructor is empty, we have to keep it in the CPP. 
		// This is because otherwise this CPP is empty, causing the RTTI registration code above to be optimized away, causing the ID property to not be registered in RTTI.
		Object::Object()
		{ }


		Object::~Object()
		{
			// We only want to reset pointers to this object if the refcount is non-zero. 
			// The reason for this is that for cases where there are objects without ObjecPtrs (for example, when deserializing objects on another thread), we want to ensure
			// we don't touch the ObjectPtrManager, as that is not thread safe. If you want to deserialize objects on other threads in a thread-safe manner, make sure to 
			// use EPointerPropertyMode::OnlyRawPointers. This will validate that there are no ObjectPtrs in any of the deserialized objects. If there are no ObjectPtrs, there
			// will be no access to ObjectPtrManager in the destructor.
			if (mObjectPtrRefCount != 0)
			{
				ObjectPtrManager::get().resetPointers(*this);
				assert(mObjectPtrRefCount == 0);
			}
		}
	}
}
