/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "rttiutilities.h"
#include "linkresolver.h"
#include "deserializeresult.h"
#include <unordered_map>

namespace nap
{
	namespace rtti
	{
		/**
		 * Default implementation of LinkResolver that can resolve UnresolvedPointer against a list of object coming out of deserialization.
		 */
		class NAPAPI DefaultLinkResolver : public LinkResolver
		{
		public:
			DefaultLinkResolver(const OwnedObjectList& objects);

			/**
			 * Convenience function that internally just constructs a DefaultLinkResolver and passes on the arguments
			 */
			static bool sResolveLinks(const OwnedObjectList& objects, const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);

		private:
			virtual Object* findTarget(const std::string& targetID) override;
			virtual EInvalidLinkBehaviour onInvalidLink(const UnresolvedPointer& unresolvedPointer) override;

		private:
			using ObjectsByIDMap = std::unordered_map<std::string, rtti::Object*>;
			ObjectsByIDMap mObjectsByID;	// Objects stored by ID, used for lookup			
		};
	}
}
