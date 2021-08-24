/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

 // Local Includes
#include "artnetreceiver.h"

// External includes
#include <utility>
#include <asio.hpp>

// nap::ArtNetReceiver run time class definition
RTTI_BEGIN_CLASS(nap::ArtNetReceiver)
RTTI_PROPERTY("Port", &nap::ArtNetReceiver::mPort, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	static asio::io_service* toService(void* ioService)
	{
		return reinterpret_cast<asio::io_service*>(ioService);
	}


	bool ArtNetReceiver::init(utility::ErrorState& errorState)
	{
		mIOServiceHandle = new asio::io_service();
		return true;
	}


	bool ArtNetReceiver::start(utility::ErrorState& errorState)
	{
		try
		{
			// Create the UDP socket server
			mListener = std::make_unique<ArtNetListener>(*this, mIOServiceHandle, mPort);

			// Run the I/O service in a thread
			mRunThread = std::thread([&] { toService(mIOServiceHandle)->run(); });
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
		if (!toService(mIOServiceHandle)->stopped())
			toService(mIOServiceHandle)->stop();

		// Join the run thread if joinable
		if (mRunThread.joinable())
			mRunThread.join();

		// Cleanup the server
		mListener = nullptr;
	}


	void ArtNetReceiver::onDestroy()
	{
		if (mIOServiceHandle != nullptr)
			delete toService(mIOServiceHandle);
	}


	void ArtNetReceiver::addEvent(ArtDmxPacketEventPtr event)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(event));
	}


	void ArtNetReceiver::consumeEvents(std::queue<ArtDmxPacketEventPtr>& outEvents)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);

		// Swap events
		outEvents.swap(mEvents);

		// Clear current queue
		std::queue<ArtDmxPacketEventPtr> empty_queue;;
		mEvents.swap(empty_queue);
	}
}
