#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 * Manages the yoctopuce library.
	 * This service doesn't initialize the library explicitly. 
	 * Initialization of the yoctopuce lib happens when connecting to a hub for the first time.
	 * This service does however explicitly shut the yoctopuce lib down to prevent memory leaks.
	 */
	class NAPAPI YoctoService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		/**
		 *	Default constructor
		 */
		YoctoService(ServiceConfiguration* configuration);

		/**
		 *	Explicitly frees the yoctopuce API
		 */
		virtual ~YoctoService() override;

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
