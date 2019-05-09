#pragma once

// Local Includes
#include "yoctoethernethub.h"

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <yocto_api.h>
#include <future>
#include <atomic>
#include <yocto_rangefinder.h>
#include <yocto_proximity.h>
#include <vector>
#include <nap/numeric.h>

namespace nap
{
	/**
	 * Base class for all Yoctopuce sensor types
	 */
	class NAPAPI BaseYoctoSensor : public Device
	{
		RTTI_ENABLE(Device)
	public:
		virtual ~BaseYoctoSensor();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the last computed sensor value	
		 */
		double getValue() const								{ return mValue; }

		/**
		 * Starts reading sensor input on a background thread
		 * @param error if the sensor can't be localized or isn't online
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops a possible thread from reading sensor values
		 */
		virtual void stop() override;
		
		/**
		 * @return if the sensor is online and able to read lux values
		 */
		bool isOnline() const								{ return mReading; }

		std::string mSensorName;				///< Property: 'Name' unique name of the sensor.
		ResourcePtr<YoctoEthernetHub> mHub;		///< Property: 'Hub' hub this sensor is connected to
		int	mDelayTime = 500;					///< Property: 'DelayTime' time the sensor should sleep in between reads (ms)
		bool mLog = false;						///< Property: 'Log' If we want to log sensor data
		int	mRetries = 10;						///< Property: 'Retries' Number of times connection is retried before exiting the loop

	protected:
		/**
		 * Must be implemented by derived classes
		 * Called by this class to find the sensor with the specific sensor name
		 * @return a valid Yocto-puce sensor, nullptr if not found
		 */
		virtual YSensor* find() = 0;

		/**
		 * Called after finding the sensor and having received a new reading
		 * Use this call to change the default behavior of the reading.
		 * ie: summing multiple values, computing the average over time etc.
		 */
		virtual double compute(double reading) 				{ return reading; }

		/**
		 * Called when this sensor starts reading values on the background thread
		 */
		virtual void onStartPolling()								{ }

		YSensor* mSensor = nullptr;						///< Pointer to the yocto-puce sensor

	private:
		std::future<void>	mMonitorTask;				///< The thread that monitor the read thread
		std::atomic<double> mValue = { -1.0 };			///< Currently sampled sensor value
		std::atomic<bool>	mReading = { false };		///< If the sensor is currently online								
		std::atomic<bool>	mStopRunning = { false };	///< If the monitoring thread should run
		std::atomic<bool>	mStopReading = { false };	///< If the poll cycle needs to stop

		/**
		 * Logs a message to the console window if the log property is true
		 */
		void logMessage(const std::string& message, bool warning = false);

		/**
		 * Runs in a background thread, keeps polling values until asked to stop
		 */
		void monitor();

		/**
		 * Starts reading values from the sensor, this function is started in
		 * a separate thread when start() is called.
		 * @param sensor the yoctopuce sensor
	 	 */
		void poll(YSensor* sensor);

		/**
		 * Sleeps the poll thread for x amount of ms
		 * @param ms the number of ms to sleep
		 */
		void sleep(int ms);
	};


	//////////////////////////////////////////////////////////////////////////
	// Default Yoctopuce sensor
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Represents a specific yocto-puce sensor.
	 * This sensor just reports back the last read value.
	 * No computation is performed on the last readout.
	 */
	template<typename T>
	class YoctoSensor : public BaseYoctoSensor
	{
		RTTI_ENABLE(BaseYoctoSensor)
	public:
		// Destructor 
		virtual ~YoctoSensor()	{ }

		/**
		 * @return type casted sensor associated with this, nullptr if not located or found!
		 */
		const T* get() const					{ return static_cast<T*>(mSensor); }

		/**
		 * @return type casted sensor associated with this, nullptr if not located or found!
		 */
		T* get()								{ return static_cast<T*>(mSensor); }

		int mBufferSize = 1;					///< Property: 'BufferSize' number of samples to average out
	protected:
		/**
		 * @return the yocto-puce sensor	
		 */
		YSensor* find()							{ return T::Find(mSensorName.c_str()); }

		/**
		 * @return the average sample value based on the number of samples in the sample buffer.
		 */
		virtual double compute(double reading) override;

		/**
		 * Clears the buffer and resets all the necessary variables
		 * This is called from the same thread as compute!
		 */
		virtual void onStartPolling() override;

	private:
		uint64	mBufferIdx = 0;						///< current index in sample buffer
		float	mAccumulatedValue= 0.0f;			///< current accumulated sensor value
		bool	mFirst = true;						///< if it's the first time we sample
		std::vector<double> mBuffer;				///< holds all the sampled values
	};

	//////////////////////////////////////////////////////////////////////////
	// Type definitions for all supported yocto-puce sensors
	//////////////////////////////////////////////////////////////////////////

	using YoctoRangeSensor		= YoctoSensor<YRangeFinder>;
	using YoctoProximitySensor	= YoctoSensor<YProximity>;


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	void nap::YoctoSensor<T>::onStartPolling()
	{
		mBuffer.clear();
		mBuffer.resize(mBufferSize, 0.0);
		mFirst = true;
		mBufferIdx = 0;
		mAccumulatedValue = 0.0;
	}


	template<typename T>
	double nap::YoctoSensor<T>::compute(double reading)
	{
		// Calculate accumulated value sensor vale
		mAccumulatedValue -= mBuffer[mBufferIdx];
		mAccumulatedValue += reading;
		mBuffer[mBufferIdx] = reading;
		if (++mBufferIdx == mBuffer.size())
		{
			mBufferIdx = 0;
			mFirst = false;
		}
		// Calculate average
		double rvalue = mFirst ? mAccumulatedValue / static_cast<double>(mBufferIdx) : mAccumulatedValue / static_cast<double>(mBuffer.size());
		return rvalue;
	}
}
