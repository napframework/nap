#pragma once

// Local Includes
#include "yoctoethernethub.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <thread>
#include <atomic>
#include <nap/signalslot.h>
#include <future>
#include <atomic>

namespace nap
{
	// Forward Declares
	class YoctoEthernetHub;

	/**
	 * A light sensor connected to a certain hub. This object does nothing on initialization
	 * The ethernet hub controls and manages a specific sensor. 
	 */
	class YoctoRangeFinder : public Device
	{
		RTTI_ENABLE(Device)
		friend class YoctoEthernetHub;
	public:
		YoctoRangeFinder();
		virtual ~YoctoRangeFinder();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return if the sensor is online and able to read lux values
		 */
		bool isOnline() const							{ return mReading; }

		/**
		 * @return the currently active value, when read-out fails this value is -1.0f
		 */
		float getValue() const							{ return mValue; }

		/**
		 * Stops a possible thread from reading sensor values
		 */
		virtual void stop() override;

		/**
		 * Starts reading sensor input on a background thread
		 * @param error if the sensor can't be localized or isn't online
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		std::string						mName;					///< Property: 'Name' name of the lux sensor
		int								mRetries = 10;			///< Property: 'Retries' Number of times connection is retried before exiting the loop
		int								mBufferSize = 30;		///< Property: 'BufferSize' of the lux sensor read-out buffer
		int								mDelayTime = 500;		///< Property: 'DelayTime' Time in between sensor reads (ms)
		bool							mLog = false;			///< Property: 'Log' If we want to log sensor data
		ResourcePtr<YoctoEthernetHub>	mHub = nullptr;			///< Property: 'Hub' The hub this sensor is connected to
	private:
		std::atomic<float>				mValue = { -1.0f };		///< Current light value
		bool							mStopReading = false;	///< Stops the thread from reading sensor values
		bool							mStopRunning = false;	///< Stops the monitor thread
		bool							mReading = false;		///< If the sensor is currently online

		/**
		 *	Monitor
		 */
		void monitor();

		/**
		 * Starts reading values from the sensor, this function is started in
		 * a separate thread when calling start()
		 */
		void read();

		// The thread that monitor the read thread
		std::future<void> mMonitorTask;

		/**
		 *	Sets the current sensor value
		 */
		void setValue(float value)					{ mValue = value; }

		/**
		 * Log a message
		 */
		void logMessage(const std::string& message, bool warning = false);
	};
}
