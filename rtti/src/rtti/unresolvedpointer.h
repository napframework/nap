#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include "rttipath.h"

// STL includes
#include <string>
#include "utility/dllexport.h"


namespace nap
{
	namespace rtti
	{
		class RTTIObject;

		/**
		 * An UnresolvedPointer represents a pointer property in a nap object that is currently unresolved (i.e. null)
		 * The information in the UnresolvedPointer can be used to look up the target and update the pointer
		 *
		 * This is an output of readJSonFile and is intended for clients to be able to resolve pointers as they see fit.
		 */
		struct NAPAPI UnresolvedPointer
		{
			UnresolvedPointer(RTTIObject* object, const rtti::RTTIPath& path, const std::string& targetID) :
				mObject(object),
				mRTTIPath(path),
				mTargetID(targetID)
			{
			}

			RTTIObject*		mObject;		// The object this pointer is on
			rtti::RTTIPath	mRTTIPath;		// RTTIPath to the pointer on <mObject>
			std::string		mTargetID;		// The ID of the target this pointer should point to
		};

		using UnresolvedPointerList = std::vector<UnresolvedPointer>;
	}
}