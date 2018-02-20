#include "yoctoluxsensor.h"

// External inclues
#include <nap/logger.h>
#include <yocto_lightsensor.h>
#include <iostream>


// nap::yoctoluxsensor run time class definition 
RTTI_BEGIN_CLASS(nap::YoctoLuxSensor)
	RTTI_PROPERTY("Name",		&nap::YoctoLuxSensor::mName,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Retries",	&nap::YoctoLuxSensor::mRetries,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	YoctoLuxSensor::~YoctoLuxSensor()			
	{
		stop();
	}


	bool YoctoLuxSensor::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool YoctoLuxSensor::start(utility::ErrorState& error)
	{
		// Stop active sensor read
		stop();

		YLightSensor* sensor = yFindLightSensor(mName.c_str());
		if (!error.check(sensor->isOnline(), "%s: sensor: %s isn't online", this->mID.c_str(), mName.c_str()))
			return false;

		// Set the sensor
		mSensor = sensor;

		// Start reading in a separate thread
		mReadThread = std::thread(std::bind(&YoctoLuxSensor::read, this));
		return true;
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

		// Keep running until a 
		while (!mStopReading && mCurrentRetries <= mRetries)
		{
			// Signal that we want to reconnect
			if (mCurrentRetries > 0)
			{
				nap::Logger::warn("retry: %d", mCurrentRetries);
			}

			// Sleep
			YRETCODE sleep = ySleep(1000, errorMsg);
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

			// Read the value
			mValue = static_cast<float>(curr_sensor->get_currentValue());
			std::cout << "Current ambient light: " << mValue << " lx\n";
			mCurrentRetries = 0;
		}
	}
}