#include "etherdreamdac.h"
#include "etherdreamservice.h"
#include <nap/logger.h>
#include <chrono>
#include <thread>
#include <utility/datetimeutils.h>

RTTI_BEGIN_CLASS(nap::EtherDreamDac)
	RTTI_PROPERTY("DacName",		&nap::EtherDreamDac::mDacName,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointRate",		&nap::EtherDreamDac::mPointRate,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AllowFailure",	&nap::EtherDreamDac::mAllowFailure,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	EtherDreamDac::EtherDreamDac(EtherDreamService& service) : mService(&service)
	{
	}

	EtherDreamDac::~EtherDreamDac()
	{
		stop();
	}

	void EtherDreamDac::stop()
	{
		// Exit write thread
		if (mIsRunning)
		{
			exitWriteThread();
			mWriteThread.join();
		}

		// Disconnect
		if (isConnected())
		{
			// Disconnect
			mService->getInterface()->disconnect(mIndex);

			// Stop writing
			nap::Logger::info("Disconnecting Etherdream DAC: %s, index: %d", mDacName.c_str(), mIndex);
			if (!mService->getInterface()->stop(mIndex))
			{
				nap::Logger::warn("Unable to stop Etherdream DAC: %s, index: %d", mDacName.c_str(), mIndex);
			}

			// Disconnect
			mService->getInterface()->disconnect(mIndex);
			mConnected = false;
		}
	}


	bool EtherDreamDac::start(utility::ErrorState& errorState)
	{
		assert(mService != nullptr);
		mIsRunning = false;
		mConnected = false;

		// Add the DAC to the system
		// If the call returns true the DAC is valid and has been added, otherwise the DAC isn't available to the current systems
		if (!mService->allocateDAC(*this))
		{
			std::string error = nap::utility::stringFormat("Unable to find Etherdream DAC with name: %s", mDacName.c_str());
			nap::Logger::warn(error.c_str());
			return errorState.check(mAllowFailure, error.c_str());
		}

		// Connect the dac
		if (!mService->getInterface()->connect(mIndex))
		{
			std::string error = nap::utility::stringFormat("Unable to connect to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
			nap::Logger::warn(error.c_str());
			return errorState.check(mAllowFailure, error.c_str());
		}

		nap::Logger::info("Successfully connected to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
		mConnected = true;

		// Start thread
		mWriteThread = std::thread(std::bind(&EtherDreamDac::writeThread, this));

		return true;
	}

	

	// Continuously writes to thread
	void EtherDreamDac::writeThread()
	{
		// We're running now
		mIsRunning = true;

		// Timer is used for checking heart-beat
		utility::SystemTimer timer;
		timer.start();

		std::vector<EtherDreamPoint> mPointsToWrite;
		while (!mStopWriting)
		{
			nap::EtherDreamInterface::EStatus write_status = getWriteStatus();
			switch (write_status)
			{
				case EtherDreamInterface::EStatus::ERROR:
				{
					nap::Logger::warn("Unable to write to DAC: %s, error occurred", mDacName.c_str());
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					break;
				}
				case EtherDreamInterface::EStatus::BUSY:
				{
					break;
				}
				case EtherDreamInterface::EStatus::READY:
				{
					// Copy points over
					mWriteMutex.lock();
					if (mPoints.size() == 0)
					{
						mWriteMutex.unlock();
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						continue;
					}
					mPointsToWrite = mPoints;
					mWriteMutex.unlock();

					// Write data
					if (!writeFrame(&(mPointsToWrite.front()), mPointsToWrite.size()))
					{
						nap::Logger::warn("Unable to write frame to Etherdream DAC: %s", mDacName.c_str());
					}
					else
					{
						// Reset timer for heart-beat
						timer.reset();
					}
					break;
				}
			}

			// Check if we have had success writing frames
			// If the time passed between the last write is higher than 3 seconds something
			// must be wrong
			if (timer.getElapsedTime() > 3.0f)
			{
				// Write frame
				nap::Logger::warn("Etherdream: %s isn't writing any new frames", mDacName.c_str());
				timer.reset();
			}
		}

		mIsRunning = false;
	}


	void EtherDreamDac::exitWriteThread()
	{
		mStopWriting = true;
	}


	nap::EtherDreamInterface::EStatus EtherDreamDac::getWriteStatus() const
	{
		return mService->getInterface()->getStatus(mIndex);
	}
	

	bool EtherDreamDac::isConnected() const
	{
		return mConnected;
	}


	void EtherDreamDac::setPoints(std::vector<EtherDreamPoint>& points)
	{
		mWriteMutex.lock();
		mPoints = points;
		mWriteMutex.unlock();
	}


	bool EtherDreamDac::writeFrame(EtherDreamPoint* data, uint npoints)
	{
		return mService->getInterface()->writeFrame(mIndex, data, npoints, static_cast<uint>(mPointRate), 1);
	}

}