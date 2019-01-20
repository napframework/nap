// Local Includes
#include "apievent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APIEvent)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS


namespace nap
{
	APIEvent::APIEvent(const std::string& action) : mName(action)
	{

	}


	APIEvent::APIEvent(const std::string&& action) : mName(std::move(action))
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
}