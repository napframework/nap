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
	RTTI_PROPERTY("Port",	&nap::OSCReceiver::mPort,	nap::rtti::EPropertyMetaData::Required)
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
		if (mSocket != nullptr)
		{
			mSocket->asynchronousBreak();
			mEventThread.join();
			nap::Logger::info("Stopped listening for OSC messages on port: %d", mPort);
		}

		// Remove from service
		mService->removeReceiver(*this);
	}


	/**
	 * Creates the thread that will run the OSC message handler
	 */
	bool OSCReceiver::init(utility::ErrorState& errorState)
	{
		// Register the receiver
		mService->registerReceiver(*this);

		// Create the socket
		mSocket = std::make_unique<OSCReceivingSocket>(IpEndpointName(IpEndpointName::ANY_ADDRESS, mPort));

		// Create and set the listener
		mListener = std::make_unique<OSCPacketListener>();
		mSocket->setListener(mListener.get());
		nap::Logger::info("Started listening for OCS messages on port: %d", mPort);

		// Create the thread and start listening for events
		mEventThread = std::thread(std::bind(&OSCReceiver::eventThread, this, mPort));
		return true;
	}
	

	void OSCReceiver::consumeEvents()
	{
		mListener->consumeEvents(mEvents);
	}


	/**
	 * This thread creates OSC connection and listener
	 * The reason why the socket is created in the thread is because the socket needs a listener
	 * I did not want to expose the OSC interface so decided to let the thread handle object creation
	 */
	void OSCReceiver::eventThread(int port)
	{
		// Create the listener
		mSocket->run();
	}
}