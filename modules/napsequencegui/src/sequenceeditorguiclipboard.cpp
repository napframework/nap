#include "sequenceeditorguiclipboard.h"

#include <nap/logger.h>
#include <rtti/jsonwriter.h>

namespace nap
{
	using namespace SequenceGUIClipboards;

	Clipboard::Clipboard(rttr::type type)
		: mSegmentType(type)
	{
	}

	void Clipboard::addObject(const rtti::Object* object, utility::ErrorState& errorState)
	{
		if(errorState.check(object->get_type() == mSegmentType, "Object is not of correct type"))
			return;

		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(rtti::ObjectList{ const_cast<rtti::Object*>(object) }, writer, errorState))
		{
			nap::Logger::error("Error serializing object %s , error : ", object->mID.c_str(), errorState.toString().c_str());
		}else
		{
			std::string serializedObject = writer.GetJSON();
			mSerializedObjects.emplace_back(serializedObject);
		}
	}
}