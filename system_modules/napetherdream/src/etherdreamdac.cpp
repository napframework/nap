/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "etherdreamdac.h"
#include "etherdreamservice.h"
#include <nap/logger.h>
#include <chrono>
#include <thread>
#include <nap/timer.h>

RTTI_BEGIN_CLASS(nap::EtherDreamDac)
	RTTI_PROPERTY("DacName",		&nap::EtherDreamDac::mDacName,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointRate",		&nap::EtherDreamDac::mPointRate,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AllowFailure",	&nap::EtherDreamDac::mAllowFailure,	    nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	EtherDreamDac::EtherDreamDac(EtherDreamService& service)
		: mService(&service), mConnected(false), mStatus(EtherDreamInterface::EStatus::ERROR)
	{
	}


	void EtherDreamDac::stop()
	{
		// If we're not connected we don't have to stop anything
		if (!mConnected)
			return;

		// Stop writing and wait for thread to finish
		if (mWriteThread.joinable())
		{
			mStopWriting = true;
			mWriteThread.join();
		}

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


	bool EtherDreamDac::start(utility::ErrorState& errorState)
	{
		assert(mService != nullptr);
		mConnected = false;

		// Add the DAC to the system
		// If the call returns true the DAC is valid and has been added, otherwise the DAC isn't available to the current systems
		if (!mService->allocateDAC(*this))
		{
			std::string error = nap::utility::stringFormat("Unable to find Etherdream DAC with name: %s", mDacName.c_str());
			nap::Logger::warn(error.c_str());
			return errorState.check(mAllowFailure, error.c_str());
		}

		// Connect the DAC, if allow failure is turned on failure to connect is allowed
		if (!mService->getInterface()->connect(mIndex))
		{
			std::string error = nap::utility::stringFormat("Unable to connect to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);
			nap::Logger::warn(error.c_str());
			return errorState.check(mAllowFailure, error.c_str());
		}

		mConnected = true;
		nap::Logger::info("Successfully connected to Etherdream DAC with name: %s, index: %d", mDacName.c_str(), mIndex);

		// Start thread
		mStopWriting = false;
		mWriteThread = std::thread(std::bind(&EtherDreamDac::writeThread, this));

		return true;
	}

	

	// Continuously writes to thread
	void EtherDreamDac::writeThread()
	{
		// Timer is used for checking heart-beat
		SystemTimer timer;
		timer.start();

		std::vector<EtherDreamPoint> mPointsToWrite;
		while (!mStopWriting)
		{
			mStatus = getWriteStatus();
			switch (mStatus)
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
	}


	nap::EtherDreamInterface::EStatus EtherDreamDac::getWriteStatus() const
	{
		return mService->getInterface()->getStatus(mIndex);
	}
	

	bool EtherDreamDac::isConnected() const
	{
		return mConnected;
	}


	nap::EtherDreamInterface::EStatus EtherDreamDac::getStatus() const
	{
		return mStatus;
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