#include "etherdreamdac.h"
#include "etherdreamservice.h"
#include <nap/logger.h>
#include <chrono>
#include <thread>
#include <nap/timer.h>

RTTI_BEGIN_CLASS(nap::EtherDreamDac)
	RTTI_PROPERTY("DacName",	&nap::EtherDreamDac::mDacName,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointRate",	&nap::EtherDreamDac::mPointRate,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	EtherDreamDac::EtherDreamDac(EtherDreamService& service) : mService(&service)
	{}

	EtherDreamDac::~EtherDreamDac()
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
		}

		// Remove dac from service
		mService->removeDAC(*this);
	}


	bool EtherDreamDac::init(utility::ErrorState& errorState)
	{
		assert(mService != nullptr);
		mIsRunning = false;

		// Add the DAC to the system
		// If the call returns true the DAC is valid and has been added, otherwise the DAC isn't
		// available to the current systems
		if (!mService->addDAC(*this))
		{
			nap::Logger::warn("Unable to find Etherdream DAC with name: %s", mDacName.c_str());
			mStatus = EConnectionStatus::UNAVAILABLE;
			return true;
		}

		// Connect the dac
		if (!mService->getInterface()->connect(mIndex))
		{
			nap::Logger::warn("Unable to connect to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
			mStatus = EConnectionStatus::CONNECTION_ERROR;
			return true;
		}

		nap::Logger::info("Successfully connected to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
		mStatus = EConnectionStatus::CONNECTED;

		// Start thread
		mWriteThread = std::thread(std::bind(&EtherDreamDac::writeThread, this));

		// Wait a bit for the connection to be established (hack)
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		return true;
	}

	

	// Continuously writes to thread
	void EtherDreamDac::writeThread()
	{
		// We're running now
		mIsRunning = true;

		SimpleTimer timer;
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
					continue;
				}
				case EtherDreamInterface::EStatus::BUSY:
				{
					continue;
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

					if (timer.getElapsedTime() > 1.0f)
					{
						// Write frame
						nap::Logger::info("still kicking it from: %s", mDacName.c_str());
						timer.reset();
					}
					writeFrame(&(mPointsToWrite.front()), mPointsToWrite.size());
					break;
				}
			}
		}

		mIsRunning = false;
	}


	void EtherDreamDac::exitWriteThread()
	{
		mStopWriting = true;
	}


	nap::EtherDreamDac::EConnectionStatus EtherDreamDac::getConnectionStatus() const
	{
		return mStatus;
	}


	nap::EtherDreamInterface::EStatus EtherDreamDac::getWriteStatus() const
	{
		return isConnected() ? mService->getInterface()->getStatus(mIndex) : EtherDreamInterface::EStatus::ERROR;
	}
	

	bool EtherDreamDac::isConnected() const
	{
		return getConnectionStatus() == EConnectionStatus::CONNECTED;
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