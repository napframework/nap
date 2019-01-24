// Local Includes
#include "apimessage.h"

// External Includes
#include <rtti/rttiutilities.h>
#include <rtti/jsonwriter.h>

// nap::apimessage run time class definition 
RTTI_BEGIN_CLASS(nap::APIMessage)
	RTTI_PROPERTY("Arguments", &nap::APIMessage::mArguments, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	APIMessage::APIMessage(const APIEvent& apiEvent) : Resource()
	{
		mID = apiEvent.getID();
		mArguments.clear();
		mOwningArguments.clear();
		for (const auto& arg : apiEvent.getArguments())
		{
			// Create copy of api value
			rtti::Variant arg_copy = arg->getValue().get_type().create();
			assert(arg_copy.is_valid());

			// Copy over all properties
			APIBaseValue* copy_ptr = arg_copy.get_value<APIBaseValue*>();
			rtti::copyObject(arg->getValue(), *copy_ptr);

			// Store
			mArguments.emplace_back(ResourcePtr<APIBaseValue>(copy_ptr));
			mOwningArguments.emplace_back(std::unique_ptr<APIBaseValue>(copy_ptr));
		}
	}


	APIMessage::~APIMessage() 
	{
		mOwningArguments.clear();
	}


	nap::APIEventPtr APIMessage::toAPIEvent()
	{
		APIEventPtr ptr = std::make_unique<APIEvent>(mID);
		for (const auto& arg : mArguments)
		{
			// Create copy using RTTR
			rtti::Variant arg_copy = arg->get_type().create();
			assert(arg_copy.is_valid());

			// Wrap result in unique ptr
			std::unique_ptr<APIBaseValue> copy(arg_copy.get_value<APIBaseValue*>());

			// Copy all properties as we know they contain the same ones
			rtti::copyObject(*arg, *copy);

			// Add API value as argument
			ptr->addArgument(std::move(copy));
		}
		return ptr;
	}


	bool APIMessage::toJSON(std::string& outString, utility::ErrorState& error)
	{
		rtti::JSONWriter writer;
		rtti::ObjectList list = { this };
		if (!serializeObjects(list, writer, error))
			return false;
		outString = writer.GetJSON();
		return true;
	}
}