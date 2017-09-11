#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>
#include <utility/dllexport.h>
#include <thread>
#include <queue>

// Local Includes
#include "oscpacketlistener.h"
#include "oscreceivingsocket.h"
#include "oscevent.h"

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
		friend class OSCService;
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

		/**
		 * Adds an event to the queue
		 * @param event the event to add, note that this receiver will take ownership of the event
		 */
		void addEvent(OSCEventPtr event);

	private:
		// Runs in the background
		void eventThread(int port);

		/**
		* Consumes all received OSC events and moves them to outEvents
		* Calling this will clear the internal queue and transfers ownership of the events to the caller
		* @param outEvents will hold the transferred osc events
		*/
		void consumeEvents(std::queue<OSCEventPtr>& outEvents);

		// The socket used for receiving messages
		std::unique_ptr<OSCReceivingSocket> mSocket = nullptr;

		// Queue that holds all the consumed events
		std::queue<OSCEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex	mEventMutex;

		// The listener used for handling messages
		std::unique_ptr<OSCPacketListener>  mListener = nullptr;

		// OSC service that manages all the osc receivers / senders
		OSCService* mService = nullptr;

		// The thread that receives and converts the messages
		std::thread mEventThread;
	};

	// Object creator used for constructing the the OSC receiver
	using OSCReceiverObjectCreator = rtti::ObjectCreator<OSCReceiver, OSCService>;
}