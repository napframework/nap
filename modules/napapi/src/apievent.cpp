// Local Includes
#include "apievent.h"

RTTI_DEFINE_BASE(nap::APIEvent)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APICallEvent)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS


namespace nap
{
	APICallEvent::APICallEvent(const std::string& action) : mName(action)
	{

	}


	APICallEvent::APICallEvent(const std::string&& action) : mName(std::move(action))
	{

	}


	APIArgument* APICallEvent::addArgument(std::unique_ptr<APIBaseValue> value)
	{
		// Create argument and move value
		std::unique_ptr<APIArgument> argument = std::make_unique<APIArgument>(std::move(value));
		mArguments.emplace_back(std::move(argument));
		return mArguments.back().get();
	}


	const nap::APIArgument* APICallEvent::getArgument(int index) const
	{
		assert(index < mArguments.size() && index >= 0);
		return mArguments[index].get();
	}


	nap::APIArgument* APICallEvent::getArgument(int index)
	{
		assert(index < mArguments.size() && index >= 0);
		return mArguments[index].get();
	}
}