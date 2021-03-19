#include "sequenceeditorguiclipboard.h"

#include <nap/logger.h>
#include <rtti/jsonwriter.h>

namespace nap
{
	using namespace SequenceGUIClipboards;

	Clipboard::Clipboard(const rttr::type& trackType)
		: mTrackType(trackType)
	{

	}

	void Clipboard::addObject(const rtti::Object* object, const std::string& sequenceName, utility::ErrorState& errorState)
	{
		// clear serialized objects if we loaded another show and add a segment from that sequence
		if(sequenceName != mSequenceName)
		{
			mSerializedObjects.clear();
		}

		// first remove the object if it already exists in the clipboard
		if(containsObject(object->mID, sequenceName))
		{
			removeObject(object->mID);
		}

		/*
		 * TODO : FIX CONST CAST
		 * I'm forced to do a const cast because the serializeObjects call does not accept a list of const object pointers
		 * This should however be possible in my view, since serialization only requires reading from the object and never writing
		 * So either we copy the object, making in non-const OR we make the serializeObjects call work with const object pointers
		 */

		// serialize the object
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(rtti::ObjectList{ const_cast<rtti::Object*>(object) }, writer, errorState))
		{
			nap::Logger::error("Error serializing object %s , error : ", object->mID.c_str(), errorState.toString().c_str());
		}else
		{
			std::string serialized_object = writer.GetJSON();
			mSerializedObjects.insert(std::pair<std::string, std::string>(object->mID, serialized_object));
		}
	}


	bool Clipboard::containsObject(const std::string& objectID, const std::string& sequenceName) const
	{
		// different sequence so, does not contain
		if( sequenceName != mSequenceName )
			return false;

		return mSerializedObjects.find(objectID) != mSerializedObjects.end();
	}


	void Clipboard::removeObject(const std::string& objectID)
	{
		auto it = mSerializedObjects.find(objectID);
		if(it!=mSerializedObjects.end())
		{
			mSerializedObjects.erase(it);
		}
	}


	std::vector<std::string> Clipboard::getObjectIDs() const
	{
		std::vector<std::string> ids;
		for(auto pair : mSerializedObjects)
		{
			ids.emplace_back(pair.first);
		}

		return ids;
	}
}