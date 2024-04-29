/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "oscreceiver.h"
#include "oscpacketlistener.h"
#include "oscreceivingsocket.h"
#include "oscservice.h"

// External 
#include <ip/UdpSocket.h>
#include <osc/OscPacketListener.h>
#include <nap/logger.h>
#include <iostream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCReceiver, "Receives OSC network messages")
	RTTI_PROPERTY("Port",				&nap::OSCReceiver::mPort,			nap::rtti::EPropertyMetaData::Required, "Port that is opened to receive osc messages")
	RTTI_PROPERTY("EnableDebugOutput",	&nap::OSCReceiver::mDebugOutput,	nap::rtti::EPropertyMetaData::Default,	"Log OSC port network information")
	RTTI_PROPERTY("AllowPortReuse",		&nap::OSCReceiver::mAllowPortReuse,	nap::rtti::EPropertyMetaData::Default,	"Don't lock the port")
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// OscReceiver
	//////////////////////////////////////////////////////////////////////////

	OSCReceiver::OSCReceiver(OSCService& service) : mService(&service)
	{	}


	/**
	 * Creates the thread that will run the OSC message handler
	 */
	bool OSCReceiver::start(utility::ErrorState& errorState)
	{
		// Register the receiver
		mService->registerReceiver(*this);

		// Create the socket, catch end point creation exception
		// We allow the try catch here because of the 3rd party lib throwing an exception.
		try
		{
			mSocket = std::make_unique<OSCReceivingSocket>(IpEndpointName(IpEndpointName::ANY_ADDRESS, mPort), mAllowPortReuse);
		}
		catch (const std::runtime_error& exception)
		{
			errorState.fail("Failed to create OSCReceiver: %s", exception.what());
			mSocket = nullptr;
			return false;
		}		

		// Create and set the listener
		mListener = std::make_unique<OSCPacketListener>(*this);
		mListener->setDebugOutput(mDebugOutput);
		mSocket->setListener(mListener.get());
		nap::Logger::info("Started listening for OSC messages on port: %d", mPort);

		// Create the thread and start listening for events
		mEventThread = std::thread(std::bind(&OSCReceiver::eventThread, this, mPort));
		return true;
	}
	

	void OSCReceiver::stop()
	{
		assert(mSocket != nullptr);
		mSocket->stop();
		mEventThread.join();
		mService->removeReceiver(*this);
		mSocket = nullptr;
		nap::Logger::info("Stopped listening for OSC messages on port: %d", mPort);
	}


	void OSCReceiver::addEvent(OSCEventPtr event)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(event));
	}


	void OSCReceiver::consumeEvents(std::queue<OSCEventPtr>& outEvents)
	{
		// Swap events
		std::lock_guard<std::mutex> lock(mEventMutex);
		outEvents.swap(mEvents);

		// Clear current queue
		mEvents.swap(std::queue<OSCEventPtr>());
	}


	void OSCReceiver::eventThread(int port)
	{
		// Starts the connection that receives osc messages
		mSocket->run();
	}
}
