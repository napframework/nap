#include "sequenceeditorguiclipboard.h"

#include <nap/logger.h>
#include <rtti/jsonwriter.h>

namespace nap
{
	using namespace SequenceGUIClipboards;

	bool Clipboard::serialize(const rtti::Object* object)
	{
		utility::ErrorState errorState;
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects(rtti::ObjectList{ const_cast<rtti::Object*>(object) }, writer, errorState))
		{
			nap::Logger::error("Error serializing object %s , error : ", object->mID.c_str(), errorState.toString().c_str());

			return false;
		}else
		{
			mSerializedObject = writer.GetJSON();
		}

		return true;
	}
}