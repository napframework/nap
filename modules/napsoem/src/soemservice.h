#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the soem (simple open ethercat master) library.
	 */
	class NAPAPI SOEMService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 * Default constructor. This service has no settings associated with it.
		 */
		SOEMService(ServiceConfiguration* configuration);
		virtual ~SOEMService() override;

	protected:
		/**
		 * Prints all the available network adapters to console.
		 * @param error contains the error if initialization fails.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& error) override;
	};
}
