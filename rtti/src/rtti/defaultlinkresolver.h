#pragma once

#include "rttiutilities.h"
#include <unordered_map>
#include "linkresolver.h"
#include "deserializeresult.h"

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

		private:
			using ObjectsByIDMap = std::unordered_map<std::string, rtti::Object*>;
			ObjectsByIDMap mObjectsByID;	// Objects stored by ID, used for lookup
		};
	}
}
