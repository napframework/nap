#pragma once

// External Includes
#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	/**
	 * Main interface for OSC messages in NAP
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