#include "yoctosensor.h"

// Extenal Includes
#include <nap/logger.h>
#include <nap/numeric.h>

// nap::yoctosensor run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BaseYoctoSensor)
	RTTI_PROPERTY("Name",		&nap::BaseYoctoSensor::mSensorName,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Hub",		&nap::BaseYoctoSensor::mHub,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("DelayTime",	&nap::BaseYoctoSensor::mDelayTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Retries",	&nap::BaseYoctoSensor::mRetries,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Log",		&nap::BaseYoctoSensor::mLog,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::YoctoRangeSensor)
	RTTI_PROPERTY("BufferSize", &nap::YoctoRangeSensor::mBufferSize,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::YoctoProximitySensor)
	RTTI_PROPERTY("BufferSize", &nap::YoctoProximitySensor::mBufferSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool BaseYoctoSensor::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool BaseYoctoSensor::start(utility::ErrorState& errorState)
	{
		// Fire off monitor thread. Automatically tries to reconnect to the sensor
		// When connection fails it waits until it tries again
		mMonitorTask = std::async(std::launch::async, std::bind(&BaseYoctoSensor::monitor, this));
		return true;
	}


	void BaseYoctoSensor::stop()
	{
		// Stop all of our polling tasks
		mStopReading = true;
		mStopRunning = true;

		// Wait for monitoring thead to exit
		if (mMonitorTask.valid())
			mMonitorTask.wait();
	}


	void BaseYoctoSensor::monitor()
	{
		while (!mStopRunning)
		{
			// Locate sensor
			mSensor = this->find();

			// Notify derived classes we started polling
			onStartPolling();

			// Poll
			poll(mSensor);
		}
	}


	void BaseYoctoSensor::poll(YSensor* sensor)
	{
		// Reset some state variables
		mStopReading = false;
		mReading = false;
		int retries	= 0;

		// Keep running until a 
		while (!mStopReading)
		{
			// Get sensor and ensure it is online
			if (sensor == nullptr || !sensor->isOnline())
			{
				if (++retries > mRetries)
				{
					logMessage("appears to be offline");
					break;
				}
				sleep(mDelayTime);
				continue;
			}

			// Reset retries
			retries = 0;

			// Read current value
			float sensor_value = static_cast<float>(sensor->get_currentValue());
			logMessage(utility::stringFormat("%.2f", sensor_value));

			// Compute actual value to store
			mValue = compute(sensor_value);

			// Note that we're on-line and have a valid value in stock
			mReading = true;

			// Sleep
			sleep(mDelayTime);
		}
	}


	void BaseYoctoSensor::sleep(int ms)
	{
		// Sleep! We need to wrap this in a try catch statement 
		// The yocto-puce library can actually thow an exception here.
		try
		{
			std::string error_msg;
			YRETCODE sleep = ySleep(static_cast<uint>(mDelayTime), error_msg);
			if (sleep != YAPI_SUCCESS)
				nap::Logger::error(error_msg);
		}
		catch (const std::exception& exception)
		{
			nap::Logger::error(exception.what());
		}
	}


	void BaseYoctoSensor::logMessage(const std::string& message, bool warning /*= false*/)
	{
		if (mLog)
		{
			if (warning)
				nap::Logger::warn("%s: sensor: %s: %s", this->mID.c_str(), mSensorName.c_str(), message.c_str());
			else
				nap::Logger::info("%s: sensor: %s: %s", this->mID.c_str(), mSensorName.c_str(), message.c_str());
		}
	}

}