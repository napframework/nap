#include "yoctoluxsensor.h"

// External inclues
#include <nap/logger.h>
#include <yocto_lightsensor.h>
#include <iostream>
#include <nap/numeric.h>

// nap::yoctoluxsensor run time class definition 
RTTI_BEGIN_CLASS(nap::YoctoLuxSensor)
	RTTI_PROPERTY("Name",		&nap::YoctoLuxSensor::mName,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Retries",	&nap::YoctoLuxSensor::mRetries,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize", &nap::YoctoLuxSensor::mBufferSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DelayTime",	&nap::YoctoLuxSensor::mDelayTime,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    YoctoLuxSensor::YoctoLuxSensor()
    {  }
    

	YoctoLuxSensor::~YoctoLuxSensor()			
	{
		stop();
	}


	bool YoctoLuxSensor::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void YoctoLuxSensor::start()
	{
		// Fire off monitor thread. Automatically tries to reconnect to the sensor
		// When connection fails it waits until it tries again
		mMonitorThread = std::thread(std::bind(&YoctoLuxSensor::monitor, this));
	}


	void YoctoLuxSensor::monitor()
	{
		while (!mStopRunning)
		{
			// Find sensor and try to locate it
			// Note that it's allowed for the sensor not to be found immediately
			// The loop that reads the value will try to connect 10 times before bumming out
			YLightSensor* sensor = yFindLightSensor(mName.c_str());

			// Set the sensor
			mSensor = sensor;

			// Start reading in a separate thread
			mReadThread = std::thread(std::bind(&YoctoLuxSensor::read, this));

			// Wait till it finishes
			mReadThread.join();

			// Check if the sensor is still online
			if (!sensor->isOnline())
			{
				nap::Logger::warn("%s: sensor: %s appears to be offline", this->mID.c_str(), mName.c_str());
			}
		}
	}


	void YoctoLuxSensor::stop()
	{
		mStopRunning = true;
		mStopReading = true;
		if (mMonitorThread.joinable())
		{
			mMonitorThread.join();
		}
	}


	void YoctoLuxSensor::read()
	{
		// Get the sensor
		assert(mSensor != nullptr);
		YLightSensor* curr_sensor = static_cast<YLightSensor*>(mSensor);

		// Reset some state variables
		mStopReading = false;
		int retries = 0;
		uint64 lux_idx = 0;
		float accum_value = 0.0f;
		bool first = true;
		std::string error_msg;

		// Create buffer that holds x amount of lux read-out values
		std::vector<float> lux_buffer(mBufferSize, 0.0f);

		// Keep running until a 
		while (!mStopReading)
		{
			// Signal that we want to reconnect
			if (retries > 0)
			{
				nap::Logger::warn("lux sensor: %s retry: %d", mID.c_str(), retries);
			}

			// Sleep
			YRETCODE sleep = ySleep(static_cast<uint>(mDelayTime), error_msg);
			if (sleep != YAPI_SUCCESS)
			{
				if(++retries > mRetries)
					break;
				continue;
			}

			// Check if the sensor is still online
			if (!curr_sensor->isOnline())
			{
				if(++retries > mRetries)
					break;
				continue;
			}

			// Read current value
			float sensor_value = static_cast<float>(curr_sensor->get_currentValue());

			// Calculate accumulated value sensor vale
			accum_value -= lux_buffer[lux_idx];
			accum_value += sensor_value;
			lux_buffer[lux_idx] = sensor_value;
			if (++lux_idx == lux_buffer.size())
			{
				lux_idx = 0;
				first = false;
			}

			// Calculate average
			mValue = first ? accum_value / static_cast<float>(lux_idx) : accum_value / static_cast<float>(lux_buffer.size());

			// Note that we're online and have a valid value in stock
			mReading = true;

			// Reset retries
			retries = 0;
		}

		// When exiting this loop we're no longer reading any values
		mReading = false;
	}
}
