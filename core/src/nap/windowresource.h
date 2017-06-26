#pragma once

#include "resource.h"
#include "event.h"

namespace nap
{
	class WindowResource : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		// Default constructor
		WindowResource() = default;
		~WindowResource();

		virtual const std::string getDisplayName() const { return ""; }
		void addEvent(EventPtr inEvent);
		void processEvents();

		Signal<const Event&> onEvent;

	private:
		using EventPtrList = std::vector<EventPtr>;
		EventPtrList mEvents;
	};
}
