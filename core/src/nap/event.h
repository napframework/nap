#pragma once

#include "rtti/rtti.h"
#include "utility/dllexport.h"

namespace nap
{
	class NAPAPI Event
	{
		RTTI_ENABLE()
	public:
		// Default construction / destruction
		virtual ~Event() = default;
		Event() = default;

		// Disable copy
		Event(Event&) = delete;
		Event& operator=(const Event&) = delete;
	};

	using EventPtr = std::unique_ptr<nap::Event>;
}
