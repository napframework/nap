#include "yoctoluxsensor.h"

// External inclues
#include <nap/logger.h>
#include <yocto_lightsensor.h>
#include <iostream>
#include <nap/configure.h>

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
    {
        mReading = false;
        mValue = -1.0f;
    }
    
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
		// Stop active sensor read
		stop();

		// Find sensor and try to locate it
		// Note that it's allowed for the sensor not to be found immediately
		// The loop that reads the value will try to connect 10 times before bumming out
		YLightSensor* sensor = yFindLightSensor(mName.c_str());
		if (!sensor->isOnline())
		{
			nap::Logger::warn("%s: sensor: %s appears to be offline", this->mID.c_str(), mName.c_str());
		}

		// Set the sensor
		mSensor = sensor;

		// Start reading in a separate thread
		mReadThread = std::thread(std::bind(&YoctoLuxSensor::read, this));
	}


	void YoctoLuxSensor::stop()
	{
		if (mSensor != nullptr)
		{
			mStopReading = true;
			mReadThread.join();
			mSensor = nullptr;
		}
	}


	void YoctoLuxSensor::read()
	{
		// Read the light value
		std::string errorMsg;

		// Get the sensor
		assert(mSensor != nullptr);
		YLightSensor* curr_sensor = static_cast<YLightSensor*>(mSensor);

		// We want to continue reading
		mStopReading = false;

		// Number of retries when read-out fails
		mCurrentRetries = 0;

		// Create buffer that holds x amount of lux read-out values
		std::vector<float> lux_buffer(mBufferSize, 0.0f);
		int lux_idx = 0;
		float accum_value = 0.0f;
		bool first = true;

		// Keep running until a 
		while (!mStopReading && mCurrentRetries <= mRetries)
		{
			// Signal that we want to reconnect
			if (mCurrentRetries > 0)
				nap::Logger::warn("retry: %d", mCurrentRetries);

			// Sleep
			YRETCODE sleep = ySleep(static_cast<uint>(mDelayTime), errorMsg);
			if (sleep != YAPI_SUCCESS)
			{
				mCurrentRetries++;
				nap::Logger::warn("unable to sleep: %s", errorMsg.c_str());
				continue;
			}

			// Check if the sensor is still online
			if (!curr_sensor->isOnline())
			{
				mCurrentRetries++;
				nap::Logger::warn("lux sensor: %s is offline", mName.c_str());
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
			mCurrentRetries = 0;
		}

		// When exiting this loop we're no longer reading any values
		mReading = false;
	}
}
