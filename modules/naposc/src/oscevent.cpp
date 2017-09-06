#include "oscevent.h"

// RTTI Definitions
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCEvent)
	RTTI_CONSTRUCTOR(const std::string&)
RTTI_END_CLASS

namespace nap
{

	OSCEvent::OSCEvent(const std::string& address) : mAddress(address)
	{

	}

}