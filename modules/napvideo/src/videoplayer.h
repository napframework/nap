#pragma once

// External Includes
#include <nap/device.h>

namespace nap
{
	/**
	 * videoplayer
	 */
	class NAPAPI VideoPlayer : public Device
	{
		RTTI_ENABLE(Device)
	public:

		// Stops the device
		virtual ~VideoPlayer() override;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the device
		 * @param errorState contains the error if the device can't be started
		 * @return if the device started
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the device
		 */
		virtual void stop() override;
	};
}
