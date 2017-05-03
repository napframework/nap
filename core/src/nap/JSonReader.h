#pragma once

// RTTI includes
#include <rtti/rtti.h>
#include "rttipath.h"

// STL includes
#include <string>

namespace nap
{
	class Object;
	struct InitResult;

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
	 * Helper struct to contain the output of readJSONFile
	 */
	struct ReadJSONFileResult
	{
		OwnedObjectList			mReadObjects;			// The list of objects that was read. Note that this struct owns these objects.
		std::vector<FileLink>	mFileLinks;				// The list of FileLinks that was read
		UnresolvedPointerList	mUnresolvedPointers;	// The list of UnresolvedPointers that was read
	};


	/**
	 * Helper functions to deserialize a JSON file or string to nap objects. This function is low level in the sense that it doesn't do any kind of pointer resolving, adding to managers, etc internally.
	 * Instead, it deserializes the JSON file and outputs the necessary information for clients to be able to do those things themselves.
	 */
	bool readJSON(const std::string& json, ReadJSONFileResult& result, nap::InitResult& initResult);
	bool readJSONFile(const std::string& filename, ReadJSONFileResult& result, nap::InitResult& initResult);	

} //< End Namespace nap

