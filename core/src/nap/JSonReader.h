#pragma once

#include <rtti/rtti.h>
#include <string>

namespace nap
{
	class Object;
	struct InitResult;

	struct UnresolvedPointer
	{
		UnresolvedPointer(Object* object, const RTTI::Property& property, const std::string& targetID);

		Object* mObject;
		RTTI::Property mProperty;
		std::string mTargetID;
	};

	struct FileLink
	{
		std::string mSourceObjectID;
		std::string mTargetFile;
	};

	using OwnedObjectList = std::vector<std::unique_ptr<nap::Object>>;
	using ObservedObjectList = std::vector<nap::Object*>;
	using UnresolvedPointerList = std::vector<UnresolvedPointer>;

	bool readJSonFile(const std::string& filename, OwnedObjectList& readObjects, std::vector<FileLink>& linkedFiles, UnresolvedPointerList& unresolvedPointers, nap::InitResult& initResult);

} //< End Namespace nap

