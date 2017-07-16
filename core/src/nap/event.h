#pragma once

#include "rtti/rtti.h"

namespace nap
{
	class Event
	{
		RTTI_ENABLE()
	};

	using EventPtr = std::unique_ptr<nap::Event>;
}
