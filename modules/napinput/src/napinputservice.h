#pragma once

// Nap Includes
#include <nap/service.h>

namespace nap
{
	class InputService;
	class WindowResource;
	class EntityInstance;
	class InputEvent;

	using EntityList = std::vector<EntityInstance*>;

	class InputRouter : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		virtual void routeEvent(const InputEvent& event, const EntityList& entities) = 0;
	};

	class DefaultInputRouter : public InputRouter
	{
		RTTI_ENABLE(InputRouter)

	public:
		virtual void routeEvent(const InputEvent& event, const EntityList& entities);
	};


	/**
	@brief nap input service
	Forwards input messages to registered input components
	**/
	class InputService : public Service
	{
		// RTTI
		RTTI_ENABLE(Service)

	public:
		// Default constructor
		InputService() = default;

		// Disable copy
		InputService(const InputService& that) = delete;
		InputService& operator=(const InputService&) = delete;
		
		void handleInput(WindowResource& window, InputRouter& inputRouter, const EntityList& entities);

	private:

	};
}