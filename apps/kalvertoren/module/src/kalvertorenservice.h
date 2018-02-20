#pragma once

// External Includes
#include <nap/service.h>

namespace nap
{
	/**
	 *	Manages yocotpuce library
	 */
	class NAPAPI KalvertorenService : public Service
	{
		RTTI_ENABLE(Service)
	public:
		KalvertorenService() = default;
		virtual ~KalvertorenService() override;

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
