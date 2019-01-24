#include "oscevent.h"

#include <nap/signalslot.h>

// RTTI Definitions
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCEvent)
	RTTI_CONSTRUCTOR(const std::string&)
    RTTI_FUNCTION("getAddress", &nap::OSCEvent::getAddress)
    RTTI_FUNCTION("getArgument", (const nap::OSCArgument*(nap::OSCEvent::*)(int)const)&nap::OSCEvent::getArgument)
    RTTI_FUNCTION("getArgumentCount", &nap::OSCEvent::getCount)
RTTI_END_CLASS

using OSCEventSignal = nap::Signal<const nap::OSCEvent&>;
RTTI_BEGIN_CLASS(OSCEventSignal)
    RTTI_FUNCTION("connect", (void(OSCEventSignal::*)(const pybind11::function))&OSCEventSignal::connect)
RTTI_END_CLASS

namespace nap
{

	OSCEvent::OSCEvent(const std::string& address) : mAddress(address)
	{

	}


	OSCEvent::OSCEvent(const std::string&& address) : mAddress(std::move(address))
	{

	}


	nap::OSCArgument* OSCEvent::addString(const std::string& string)
	{
		return addArgument<nap::OSCString>(string);
	}


	const OSCArgument* OSCEvent::getArgument(int index) const
	{
		assert(index < mArguments.size() && index >= 0);
		return mArguments[index].get();
	}


	nap::OSCArgument* OSCEvent::getArgument(int index)
	{
		assert(index < mArguments.size() && index >= 0);
		return mArguments[index].get();
	}


	std::size_t OSCEvent::getSize() const
	{
		std::size_t event_size(0);
		for (const auto& arg : mArguments)
		{
			event_size += arg->size();
		}
		return event_size + mAddress.size();
	}
}
