#pragma once

// External Includes
#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	/**
	 * Main interface for rendering to various Etherdream Dacs
	 * The service is responsible for opening / closing the general Etherdream library
	 * and allows for rendering data to the available dacs
	 */
	class NAPAPI OSCService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		OSCService() = default;

		// Default Destructor
		virtual ~OSCService();

		// Initialization
		bool init(nap::utility::ErrorState& errorState);
	};
}