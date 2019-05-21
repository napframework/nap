#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the serial library.
	 */
	class NAPAPI SerialService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		SerialService(ServiceConfiguration* configuration);

		/**
		 *	Explicitly frees the yoctopuce API
		 */
		virtual ~SerialService() override;

	protected:
		/**
		 * Initialize yoctopuce related functionality
		 */
		virtual bool init(utility::ErrorState& error) override;

		/**
		 *	Shuts down all yoctopuce related functionality
		 */
		virtual void shutdown() override;
	};
}
