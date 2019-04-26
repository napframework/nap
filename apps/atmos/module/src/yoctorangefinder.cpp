#include "yoctorangefinder.h"

// External inclues
#include <nap/logger.h>
#include <yocto_rangefinder.h>
#include <iostream>
#include <nap/numeric.h>
#include <future>
#include <nap/timer.h>

// nap::yoctoluxsensor run time class definition 
RTTI_BEGIN_CLASS(nap::YoctoRangeFinder)
	RTTI_PROPERTY("Name",		&nap::YoctoRangeFinder::mName,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Retries",	&nap::YoctoRangeFinder::mRetries,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BufferSize", &nap::YoctoRangeFinder::mBufferSize,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DelayTime",	&nap::YoctoRangeFinder::mDelayTime,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Log",		&nap::YoctoRangeFinder::mLog,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Hub",		&nap::YoctoRangeFinder::mHub,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    YoctoRangeFinder::YoctoRangeFinder()
    {  }
    

	YoctoRangeFinder::~YoctoRangeFinder()			
	{
		stop();
	}


	bool YoctoRangeFinder::init(utility::ErrorState& errorState)
	{
		return true;
	}


	bool YoctoRangeFinder::start(utility::ErrorState& errorState)
	{
		// Fire off monitor thread. Automatically tries to reconnect to the sensor
		// When connection fails it waits until it tries again
		mMonitorTask = std::async(std::launch::async, std::bind(&YoctoRangeFinder::monitor, this));
		return true;
	}


	void YoctoRangeFinder::monitor()
	{
		while (!mStopRunning)
			read();
	}


	void YoctoRangeFinder::stop()
	{
		// Stop all of our polling tasks
		mStopReading = true;
		mStopRunning = true;

		// Wait for monitoring thead to exit
		if(mMonitorTask.valid())
			mMonitorTask.wait();
	}


	void YoctoRangeFinder::read()
	{
		// Reset some state variables
		mStopReading = false;
		mReading = false;
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
			try
			{
				// Sleep
				YRETCODE sleep = ySleep(static_cast<uint>(mDelayTime), error_msg);
				if (sleep != YAPI_SUCCESS)
				{
					if (++retries > mRetries)
					{
						logMessage(error_msg.c_str());
						break;
					}
					continue;
				}
			}
			catch (const std::exception& exception)
			{
				nap::Logger::error(exception.what());
				break;
			}

			// Get sensor and ensure it's online
			YRangeFinder* curr_sensor = yFindRangeFinder(mName.c_str());
			if (curr_sensor == nullptr || !curr_sensor->isOnline())
			{
				if (++retries > mRetries)
				{
					logMessage("appears to be offline");
					break;
				}
				continue;
			}

			// Read current value
			float sensor_value = static_cast<float>(curr_sensor->get_currentValue());
			logMessage(utility::stringFormat("%.2f range", sensor_value));


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


	void YoctoRangeFinder::logMessage(const std::string& message, bool warning)
	{
		if (mLog)
		{
			if (warning)
				nap::Logger::warn("%s: sensor: %s: %s", this->mID.c_str(), mName.c_str(), message.c_str());
			else
				nap::Logger::info("%s: sensor: %s: %s", this->mID.c_str(), mName.c_str(), message.c_str());
		}
	}
}