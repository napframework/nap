/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "defaultlinkresolver.h"
#include "object.h"

namespace nap
{
	namespace rtti
	{
		DefaultLinkResolver::DefaultLinkResolver(const OwnedObjectList& objects)
		{
			for (auto& object : objects)
				mObjectsByID.insert({ object->mID, object.get() });
		}


		/**
		 * Resolves the target ID to an object, using the ObjectsByID mapping
		 */
		Object* DefaultLinkResolver::findTarget(const std::string& targetID)
		{
			ObjectsByIDMap::iterator pos = mObjectsByID.find(targetID);
			if (pos == mObjectsByID.end())
				return nullptr;

			return pos->second;
		}

		
		bool DefaultLinkResolver::sResolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState)
		{
			DefaultLinkResolver resolver(objects);
			return resolver.resolveLinks(unresolvedPointers, errorState);
		}
		

		LinkResolver::EInvalidLinkBehaviour DefaultLinkResolver::onInvalidLink(const UnresolvedPointer& unresolvedPointer)
		{
			return LinkResolver::EInvalidLinkBehaviour::TreatAsError;
		}
	}
}