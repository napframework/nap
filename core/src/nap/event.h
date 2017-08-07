#pragma once

#include "rtti/rtti.h"
#include "utility/dllexport.h"

namespace nap
{
	class NAPAPI Event
	{
		RTTI_ENABLE()
	public:
		virtual ~Event() = default;
		Event() = default;
	};

	using EventPtr = std::unique_ptr<nap::Event>;
}
