#pragma once

#include "resource.h"
#include "event.h"
#include "utility/uniqueptrvectoriterator.h"

namespace nap
{
	class WindowResource : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		using EventPtrList = std::vector<EventPtr>;
		using EventPtrConstIterator = utility::UniquePtrConstVectorWrapper<EventPtrList, Event*>;

		virtual const std::string getDisplayName() const { return ""; }
		void addEvent(EventPtr inEvent);
		void processEvents();

		Signal<const Event&> onEvent;
		EventPtrConstIterator GetEvents() const { return EventPtrConstIterator(mEvents); }

	private:		
		EventPtrList mEvents;
	};
}
