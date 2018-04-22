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

RTTI_BEGIN_CLASS(nap::OSCReceiver)
	RTTI_PROPERTY("Port",				&nap::OSCReceiver::mPort,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EnableDebugOutput",	&nap::OSCReceiver::mDebugOutput,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// OscReceiver
	//////////////////////////////////////////////////////////////////////////

	OSCReceiver::OSCReceiver(OSCService& service) : mService(&service)
	{	}


	OSCReceiver::~OSCReceiver()
	{
		stop();
	}

	/**
	 * Creates the thread that will run the OSC message handler
	 */
	bool OSCReceiver::start(utility::ErrorState& errorState)
	{
		// Register the receiver
		mService->registerReceiver(*this);

		// Create the socket
		mSocket = std::make_unique<OSCReceivingSocket>(IpEndpointName(IpEndpointName::ANY_ADDRESS, mPort));

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
		if (mSocket != nullptr)
		{
			mSocket->stop();
			mEventThread.join();
			mService->removeReceiver(*this);
			nap::Logger::info("Stopped listening for OSC messages on port: %d", mPort);
		}
	}


	void OSCReceiver::addEvent(OSCEventPtr event)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);
		mEvents.emplace(std::move(event));
	}

	void OSCReceiver::consumeEvents(std::queue<OSCEventPtr>& outEvents)
	{
		std::lock_guard<std::mutex> lock(mEventMutex);

		// Swap events
		outEvents.swap(mEvents);

		// Clear current queue
        std::queue<OSCEventPtr> empty_queue;;
		mEvents.swap(empty_queue);
	}


	/**
	 * Starts the connection that receives osc messages
	 */
	void OSCReceiver::eventThread(int port)
	{
		// Create the listener
		mSocket->run();
	}
}
