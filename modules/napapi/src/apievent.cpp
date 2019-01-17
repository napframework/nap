// Local Includes
#include "apievent.h"

RTTI_DEFINE_BASE(nap::APIEvent)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::APICallEvent)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS


namespace nap
{
	APICallEvent::APICallEvent(const std::string& action) : mActionName(action)
	{

	}


	APICallEvent::APICallEvent(const std::string&& action) : mActionName(std::move(action))
	{

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