/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "artnetreceiver.h"
#include "artnetservice.h"

// External includes
#include <utility>

// nap::ArtNetReceiver run time class definition
RTTI_BEGIN_CLASS(nap::ArtNetReceiver)
	RTTI_PROPERTY("IP Address", &nap::ArtNetReceiver::mIpAddress, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port", &nap::ArtNetReceiver::mPort, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	ArtNetReceiver::ArtNetReceiver(ArtNetService& service) : mService(&service)
	{

	}


	bool ArtNetReceiver::start(utility::ErrorState& errorState)
	{
		// Add receiver
		if (!mService->addReceiver(*this, errorState))
			return false;

		// ASIO can throw exceptions, i.e. when the port is in use
		try
		{
			// Create the UDP socket server
			mListener = std::make_unique<ArtNetListener>(*this, mIOService, mIpAddress, mPort);

			// Run the I/O service in a thread
			mRunThread = std::thread([&] { mIOService.run(); });
		}
		catch (std::exception& e)
		{
			// Catch possible ASIO exceptions
			errorState.fail("%s: failed to start UDP socket server: %s", mID.c_str(), e.what());
			return false;
		}

		return true;
	}


	void ArtNetReceiver::stop()
	{
		// Stop the I/O service if running
		if (!mIOService.stopped())
			mIOService.stop();

		// Join the run thread if joinable
		if (mRunThread.joinable())
			mRunThread.join();

		// Cleanup the server
		mListener = nullptr;

		// Remove controller from service
		mService->removeReceiver(*this);
	}


	void ArtNetReceiver::addEvent(ArtNetEventPtr event)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(event));
	}


	void ArtNetReceiver::consumeEvents(std::queue<ArtNetEventPtr>& outEvents)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);

		// Swap events
		outEvents.swap(mEvents);

		// Clear current queue
		std::queue<ArtNetEventPtr> empty_queue;;
		mEvents.swap(empty_queue);
	}
}
