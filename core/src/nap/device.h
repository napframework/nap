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
	 */
	class NAPAPI Device : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		 * Start the device. Called after initialization.
		 * When called it is safe to assume that all dependencies have been resolved up to this point.
		 * @param errorState The error state
		 * @return: true on success
		 */
		virtual bool start(utility::ErrorState& errorState) { return true; }

		/**
		 * Called when the device needs to be stopped, but only if start has previously been called on this Device. 
		 * It is safe to assume that when stop is called the device is in a 'started' state. Called in reverse init order.
		 */
		virtual void stop() {}
	};
}
