#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/objectptr.h>
#include <thread>
#include <atomic>

namespace nap
{
	// Forward Declares
	class YoctoEthernetHub;

	/**
	 * A light sensor connected to a certain hub. This object does nothing on initialization
	 * The ethernet hub controls and manages a specific sensor. 
	 */
	class YoctoLuxSensor : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
		friend class YoctoEthernetHub;
	public:
		YoctoLuxSensor();
		virtual ~YoctoLuxSensor();

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
		float getValue()								{ return mValue; }

		std::string			mName;						///< Property: 'Name' name of the lux sensor
		int					mRetries = 10;				///< Number of times connection is retried before exiting the loop
		int					mBufferSize = 30;			///< Size of the lux sensor read-out buffer
		int					mDelayTime = 250;			///< Time in between sensor reads

	private:
		void*				mSensor = nullptr;			///< Light sensor
		std::atomic<float>	mValue;                     ///< Current light value
		bool				mStopReading = false;		///< Stops the thread from reading sensor values
		int					mCurrentRetries = 0;		///< Number of retries associated with read out failure
		std::atomic<bool>	mReading;                   ///< If the sensor is currently online

		/**
		 * Starts reading sensor input on a background thread
		 * @param error if the sensor can't be localized or isn't online
		 * 
		 */
		void start();

		/**
		 * Stops a possible thread from reading sensor values
		 */
		void stop();

		/**
		 * Starts reading values from the sensor, this function is started in
		 * a separate thread when calling start()
		 */
		void read();

		// The thread that receives and converts the messages
		std::thread mReadThread;

		/**
		 *	Sets the current sensor value
		 */
		void setValue(float value)					{ mValue = value; }
	};
}
