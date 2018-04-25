#pragma once

// Nap Includes
#include <nap/service.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Extracts input events from Window and forwards them to an InputRouter.
	 */
	class NAPAPI SandboxService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default constructor
		SandboxService(ServiceConfiguration* configuration);

		// Disable copy
        SandboxService(const SandboxService& that) = delete;
        SandboxService& operator=(const SandboxService&) = delete;
	};
}