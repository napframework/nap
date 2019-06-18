// Local Includes
#include "apievent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIEvent)
	RTTI_CONSTRUCTOR(const std::string&)
	RTTI_CONSTRUCTOR(const std::string&, const std::string&)
RTTI_END_CLASS


namespace nap
{
	APIEvent::APIEvent(const std::string& name) : mName(name), 
		mID(math::generateUUID())
	{

	}


	APIEvent::APIEvent(std::string&& name) : mName(std::move(name)), 
		mID(math::generateUUID())
	{

	}


	APIEvent::APIEvent(const std::string& name, const std::string& id) : mName(name), 
		mID(id)
	{

	}


	APIEvent::APIEvent(std::string&& name, std::string&& id) : mName(std::move(name)), 
		mID(std::move(id))
	{

	}

	APIArgument* APIEvent::addArgument(std::unique_ptr<APIBaseValue> value)
	{
		// Create argument and move value
		std::unique_ptr<APIArgument> argument = std::make_unique<APIArgument>(std::move(value));
		mArguments.emplace_back(std::move(argument));
		return mArguments.back().get();
	}


	const nap::APIArgument* APIEvent::getArgument(int index) const
	{
		assert(index < mArguments.size() && index >= 0);
		return mArguments[index].get();
	}


	nap::APIArgument* APIEvent::getArgument(int index)
	{
		assert(index < mArguments.size() && index >= 0);
		return mArguments[index].get();
	}


	bool APIEvent::matches(const nap::APISignature& signature) const
	{
		// Make sure number of arguments is the same
		if (getCount() != signature.getCount())
			return false;

		// Make sure that every api value is of the same type
		for (int i = 0; i < getCount(); i++)
		{
			rtti::TypeInfo given_type = mArguments[i]->getValue().get_type();
			rtti::TypeInfo signa_type = signature.getValue(i).get_type();
			if (!given_type.is_derived_from(signa_type))
				return false;
		}
		return true;
	}
}