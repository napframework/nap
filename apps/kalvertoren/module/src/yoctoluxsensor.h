#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <thread>

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
		YoctoLuxSensor() = default;
		virtual ~YoctoLuxSensor();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the currently active value, when read-out fails this value is -1.0f
		 */
		float getValue()								{ return mValue; }

		std::string		mName;							///< Property: 'Name' name of the lux sensor
		int				mRetries = 10;				///< Number of times connection is retried before exiting the loop

	private:
		void* mSensor = nullptr;						///< Light sensor
		float mValue  = -1.0f;							///< Current light value
		bool  mStopReading = false;						///< Stops the thread from reading sensor values
		int	  mCurrentRetries = 0;						///< Number of retries associated with read out failure

		/**
		 * Starts reading sensor input on a background thread
		 * @param error if the sensor can't be localized or isn't online
		 * @return if the sensor was found and value read-out commenced
		 * 
		 */
		bool start(utility::ErrorState& error);

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
	};
}
