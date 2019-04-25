#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include "path.h"

// STL includes
#include <string>
#include "utility/dllexport.h"


namespace nap
{
	namespace rtti
	{
		class Object;

		/**
		 * An UnresolvedPointer represents a pointer property in a nap object that is currently unresolved (i.e. null)
		 * The information in the UnresolvedPointer can be used to look up the target and update the pointer
		 *
		 * This is an output of readJSonFile and is intended for clients to be able to resolve pointers as they see fit.
		 */
		struct NAPAPI UnresolvedPointer
		{
			UnresolvedPointer(Object* object, const rtti::Path& path, const std::string& targetID) :
				mObject(object),
				mRTTIPath(path),
				mTargetID(targetID)
			{
			}

			/**
			 * Get the ID of the resource this UnresolvedPointer is pointing to. When this is a plain unresolved pointer, this will simply
			 * return mTargetID. If this is a pointer that has a translateTargetID function (for example, ComponentPtr), it will be invoked to 
			 * translate the source ID (i.e. component path) to the resource ID.
			 *
			 * @return the ID of the resource this UnresolvedPointer is pointing to
			 */
			std::string getResourceTargetID() const;

			Object*		mObject;		// The object this pointer is on
			rtti::Path	mRTTIPath;		// RTTIPath to the pointer on <mObject>
			std::string	mTargetID;		// The ID of the target this pointer should point to
		};

		using UnresolvedPointerList = std::vector<UnresolvedPointer>;
	}
}