#pragma once

#include <nap/event.h>

namespace nap
{
	class BrowserEvent : public Message
	{
		RTTI_ENABLE_DERIVED_FROM(Message)
	public:
		BrowserEvent() = default;
		BrowserEvent(const std::string& inMethod) : Message(""), mMethod(inMethod)	{ }

		// Method from which the call originated
		std::string mMethod;
	};
}
RTTI_DECLARE_BASE(nap::BrowserEvent)
