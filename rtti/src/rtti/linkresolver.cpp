/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "linkresolver.h"
#include "object.h"

// External Includes
#include <utility/errorstate.h>

namespace nap
{
	namespace rtti
	{
		bool LinkResolver::resolveLinks(const UnresolvedPointerList& unresolvedPointers, utility::ErrorState& errorState)
		{
			for (const UnresolvedPointer& unresolved_pointer : unresolvedPointers)
			{
				std::string target_id = unresolved_pointer.getResourceTargetID();

				// Objects in objectsToUpdate have preference over the manager's objects. 
				Object* target_object = findTarget(target_id);

				if (target_object == nullptr)
				{
					if (onInvalidLink(unresolved_pointer) == EInvalidLinkBehaviour::TreatAsError)
					{
						errorState.fail("Unable to resolve link to object %s from attribute %s", target_id.c_str(), unresolved_pointer.mRTTIPath.toString().c_str());
						return false;
					}
					else
					{
						continue;
					}
				}

				assert(target_object != nullptr);

				ResolvedPath resolved_path;
				if (!errorState.check(unresolved_pointer.mRTTIPath.resolve(unresolved_pointer.mObject, resolved_path), "Failed to resolve RTTIPath %s", unresolved_pointer.mRTTIPath.toString().c_str()))
					return false;

				TypeInfo resolved_path_type = resolved_path.getType();
				TypeInfo actual_type = resolved_path_type.is_wrapper() ? resolved_path_type.get_wrapped_type() : resolved_path_type;

				if (!errorState.check(target_object->get_type().is_derived_from(actual_type), "Failed to resolve pointer: target of pointer {%s}:%s is of the wrong type (found '%s', expected '%s')",
					unresolved_pointer.mObject->mID.c_str(), unresolved_pointer.mRTTIPath.toString().c_str(), target_object->get_type().get_name().data(), actual_type.get_raw_type().get_name().data()))
				{
					return false;
				}

				assert(actual_type.is_pointer());

				// If the type that we're processing has a function to assign the pointer value, we use it.
				Variant target_value = target_object;
				rttr::method assign_method = findMethodRecursive(resolved_path.getType(), method::assign);
				if (assign_method.is_valid())
				{
					target_value = resolved_path.getValue();
					assign_method.invoke(target_value, unresolved_pointer.mTargetID, *target_object);
				}

				bool succeeded = resolved_path.setValue(target_value);
				if (!errorState.check(succeeded, "Failed to resolve pointer for: %s", target_object->mID.c_str()))
					return false;
			}

			return true;
		}
	}
}
