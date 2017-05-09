#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include "rttipath.h"

// STL includes
#include <string>


namespace nap
{
	class Object;
	class ErrorState;

	/**
	 * An UnresolvedPointer represents a pointer property in a nap object that is currently unresolved (i.e. null)
	 * The information in the UnresolvedPointer can be used to look up the target and update the pointer
	 *
	 * This is an output of readJSonFile and is intended for clients to be able to resolve pointers as they see fit.
	 */
	struct UnresolvedPointer
	{
		UnresolvedPointer(Object* object, const RTTI::RTTIPath& path, const std::string& targetID) :
			mObject(object),
			mRTTIPath(path),
			mTargetID(targetID)
		{
		}

		Object*			mObject;		// The object this pointer is on
		RTTI::RTTIPath	mRTTIPath;		// RTTIPath to the pointer on <mObject>
		std::string		mTargetID;		// The ID of the target this pointer should point to
	};


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

	using OwnedObjectList		= std::vector<std::unique_ptr<nap::Object>>;
	using ObservedObjectList	= std::vector<nap::Object*>;
	using UnresolvedPointerList = std::vector<UnresolvedPointer>;

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

