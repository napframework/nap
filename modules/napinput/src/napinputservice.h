#pragma once

// Nap Includes
#include <nap/service.h>

namespace nap
{
	class InputService;
	class WindowResource;
	class EntityInstance;

	/**
	@brief nap input service
	Forwards input messages to registered input components
	**/
	class InputService : public Service
	{
		// RTTI
		RTTI_ENABLE(Service)

	public:
		using EntityList = std::vector<EntityInstance*>;

		// Default constructor
		InputService() = default;

		// Disable copy
		InputService(const InputService& that) = delete;
		InputService& operator=(const InputService&) = delete;
		
		void handleInput(WindowResource& window, const EntityList& entities);

	private:

	};
}