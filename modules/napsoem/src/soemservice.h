#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the serial library.
	 */
	class NAPAPI SOEMService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		SOEMService(ServiceConfiguration* configuration);

		/**
		 *	Explicitly frees the serial API
		 */
		virtual ~SOEMService() override;

	protected:
		/**
		 * Initialize serial related functionality
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Shuts down all serial related functionality
		 */
		virtual void shutdown() override;
	};
}
