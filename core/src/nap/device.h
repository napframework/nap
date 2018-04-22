#pragma once

#include "resource.h"

namespace nap
{
	/**
	 * A device is a special class of resource, intended to deal with cases where there are only a limited number of them allowed to be in existence at the same time.
	 * For example, if you have a device that can only be opened by a single client, it's possible for the 'open' of the device to fail in case of real-time editing.
	 * This is because during real-time edit, it's possible for the object being edited to (temporarily) have multiple 'instances'.
	 *
	 * The Device class deals with this case by providing an explicit start/stop virtual, which are called at appropriate times by the ResourceManager.
	 * It's important that both start & stop can be called multiple times, but note that they will always be called in pairs.
	 *
	 * The Device is automatically stopped (by calling stop()) during destruction of the Device.
	 */
	class NAPAPI Device : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		 * Start the device. Will be called after init()
		 *
		 * @param errorState The error state
		 * @return: true on success
		 */
		virtual bool start(utility::ErrorState& errorState) { return true; }

		/**
		 * Stop the device. Will be called before the object is reloaded and during shutdown
		 */
		virtual void stop() {}
	};
}
