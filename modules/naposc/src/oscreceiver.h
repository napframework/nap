#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>
#include <utility/dllexport.h>
#include <thread>

// Local Includes
#include "oscpacketlistener.h"
#include "oscreceivingsocket.h"

// Forward Declares
namespace nap
{
	class OSCService;
}

namespace nap
{
	/**
	 * Object that receives and processes osc events
	 * The receiver manages it's own connection in a background thread
	 */
	class NAPAPI OSCReceiver : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		// Default constructor
		OSCReceiver() = default;

		// Constructor used by factory
		OSCReceiver(OSCService& service);

		// Kills connection
		virtual ~OSCReceiver();

		/**
		 * Initializes the receiver and registers it with the OSCService
		 * This call will also start the background thread that received the messages
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		// Property: the port to listen to for messages
		int mPort = 7000;

	private:
		// OSC service that manages all the osc receivers / senders
		OSCService* mService = nullptr;

		// The thread that receives and converts the messages
		std::thread mEventThread;

		// Runs in the background
		void eventThread(int port);

		// The socket used for receiving messages
		std::unique_ptr<OSCReceivingSocket> mSocket = nullptr;

		// The listener used for handling messages
		std::unique_ptr<OSCPacketListener>  mListener = nullptr;
	};

	// Object creator used for constructing the the OSC receiver
	using OSCReceiverObjectCreator = rtti::ObjectCreator<OSCReceiver, OSCService>;
}