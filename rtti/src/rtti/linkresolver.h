#pragma once

#include "unresolvedpointer.h"
#include <utility/dllexport.h>

namespace nap
{
	namespace utility
	{
		class ErrorState;
	}

	namespace rtti
	{
		/**
		 * Base class for resolving links after deserialization. The base class contains logic for determining target ID. The derived class should perform the lookup
		 * of the target object in its data structures.
		 * Custom pointers are supported through exposed RTTI functions on pointer objects: a static function 'translateTargetID' will convert any ID to a target ID
		 * and the member function 'assign' will assign the pointer value to the pointer object.
		 */
		class NAPAPI LinkResolver
		{
		public:
			virtual ~LinkResolver() = default;

			/**
			 * Performs the resolving of pointers. Will call findTarget for each pointer.
			 * @param unresolvedPointers The pointers to resolve
			 * @param errorState Contains error information if the function returns false.
			 * @return true if link resolution succeeded, false if not.
			 */
			bool resolveLinks(const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState);

		protected:
			/**
			 * Called by resolveLinks to resolve a target ID to target object. Derived classes should implement this to perform the lookup.
			 * @param targetID The ID to resolve. This ID could be retrieved straight from deserialization or transformed through the static translateTargetID function on the type.
			 * @return nullptr if the target could not be resolved, otherwise the resolved target object.
			 */
			virtual RTTIObject* findTarget(const std::string& targetID) = 0;
		};
	}
}
