#include "oscevent.h"

#include <nap/signalslot.h>

// RTTI Definitions
RTTI_BEGIN_CLASS(nap::OSCEvent)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS

//RTTI_BEGIN_CLASS(nap::Signal<const nap::OSCEvent&>)
//    RTTI_FUNCTION("connect", &nap::Signal<const nap::OSCEvent&>::connect)
//RTTI_END_CLASS

using OSCEventSignal = nap::Signal<nap::OSCEvent&>;
RTTI_BEGIN_CLASS(OSCEventSignal)
    RTTI_FUNCTION("connect", (void(OSCEventSignal::*)(const OSCEventSignal::Function&))&OSCEventSignal::connect)
RTTI_END_CLASS

namespace nap
{

	OSCEvent::OSCEvent(const std::string& address) : mAddress(address)
	{

	}


	nap::OSCArgument* OSCEvent::addString(const std::string& string)
	{
		return addArgument<nap::OSCString>(string);
	}


	const OSCArgument& OSCEvent::getArgument(int index) const
	{
		assert(index < mArguments.size() && index >= 0);
		return *(mArguments[index]);
	}


	nap::OSCArgument& OSCEvent::getArgument(int index)
	{
		assert(index < mArguments.size() && index >= 0);
		return *(mArguments[index]);
	}


	std::size_t OSCEvent::getSize() const
	{
		std::size_t event_size(0);
		for (const auto& arg : mArguments)
		{
			event_size += arg->size();
		}
		return event_size + mAddress.length();
	}
}
