#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include "rttipath.h"

// STL includes
#include <string>
#include "unresolvedpointer.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;

		/**
		 * Represents a file link from an object to a target file.
		 *
		 * This is an output of readJSonFile and can be used to determine file dependencies
		 */
		struct FileLink
		{
			std::string		mSourceObjectID;	// The ID of the object that has the file link
			std::string		mTargetFile;		// The path to the file that's being to
		};

		using OwnedObjectList = std::vector<std::unique_ptr<rtti::RTTIObject>>;
		using ObservedObjectList = std::vector<rtti::RTTIObject*>;

		/**
		 * Output of RTTI deserialization (both binary and json)
		 */
		struct RTTIDeserializeResult
		{
			OwnedObjectList			mReadObjects;			// The list of objects that was read. Note that this struct owns these objects.
			std::vector<FileLink>	mFileLinks;				// The list of FileLinks that was read
			UnresolvedPointerList	mUnresolvedPointers;	// The list of UnresolvedPointers that was read
		};

	} //< End Namespace nap

}
