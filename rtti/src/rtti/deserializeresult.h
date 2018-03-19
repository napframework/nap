#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include <rtti/rttiutilities.h>
#include "path.h"
#include "utility/dllexport.h"
#include "unresolvedpointer.h"

// STL includes
#include <string>

namespace nap
{
	namespace rtti
	{
		// Forward Declares
		class Object;

		/**
		 * Represents a file link from an object to a target file.
		 * This is an output of readJSonFile and can be used to determine file dependencies
		 */
		struct NAPAPI FileLink
		{
			std::string		mSourceObjectID;	// The ID of the object that has the file link
			std::string		mTargetFile;		// The path to the file that's being to
		};

		using OwnedObjectList = std::vector<std::unique_ptr<rtti::Object>>;
		using ObservedObjectList = std::vector<rtti::Object*>;

		/**
		 * Represents the output of an rtti de serialization process.
		 * This result is used by the binary and json readers
		 */
		struct NAPAPI DeserializeResult
		{
			DeserializeResult() = default;
			DeserializeResult(const DeserializeResult&) = delete;
			DeserializeResult& operator=(const DeserializeResult&) = delete;

			OwnedObjectList			mReadObjects;			// The list of objects that was read. Note that this struct owns these objects.
			std::vector<FileLink>	mFileLinks;				// The list of FileLinks that was read
			UnresolvedPointerList	mUnresolvedPointers;	// The list of UnresolvedPointers that was read
		};

	} //< End Namespace nap

}
