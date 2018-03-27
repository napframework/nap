#pragma once

#include "resource.h"

namespace nap
{
	/**
	 * 
	 */
	class NAPAPI Device : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		virtual ~Device() override;

		/**
		 * Start the device. Will be called after init()
		 *
		 * @param errorState The error state
		 * @return: true on success
		 */
		virtual bool start(utility::ErrorState& errorState) { return true; }

		/**
		 * Stop the device. Will be called before the object is reloaded
		 */
		virtual void stop() {}
	};
}
