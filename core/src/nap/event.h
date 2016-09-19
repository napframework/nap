#pragma once

#include <nap/attributeobject.h>
#include <nap/coremodule.h>
#include <rtti/rtti.h>
#include "objectutils.h"

namespace nap
{ 
	class EventDispatcher;

	// Simple container used for dispatching messages in the system
	class Event : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
	public:
		// Construction / Destruction
		Event() = default;
		virtual ~Event() = default;

		// Disable copy
		Event(const Event& that) = delete;
		Event& operator=(const Event&) = delete;

		// Event handling interface
		void setHandled(bool inValue) { mHandled = inValue; }
		bool getHandled() const { return mHandled; }

		// Source handling interface
		void setSource(const nap::Object& inSource) { source = &inSource; }
		const nap::Object* getSource() const { return source; }

	protected:
		bool mHandled = false;				 //< If the event is handled by the system
		const nap::Object* source = nullptr; //< Source of the event
	};


	/**
	@brief Event

	Event that carries a message as string
	**/
	class Message : public Event
	{
		RTTI_ENABLE_DERIVED_FROM(Event)

	public:
		// Construction
		Message(const std::string& m) { setMessage(m); }

		// Message handling
		void setMessage(const std::string& m) { message.setValue(m); }
		const std::string& getMessage() const { return message.getValue(); }
		
		Attribute<std::string> message = { this, "Message" };

	};


	/**
	@brief Event

	Event that carries a message as string
	**/
	class EventHandlerComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
	public:
		// Override the handle process incoming events
		// The return value specifies if the event should not be dispatched any further
		virtual bool handleEvent(Event& event) = 0;
	};
}

RTTI_DECLARE_BASE(nap::EventHandlerComponent)
RTTI_DECLARE_BASE(nap::Event);
RTTI_DECLARE_BASE(nap::Message);