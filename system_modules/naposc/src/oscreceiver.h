/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/device.h>
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
	 * Object that receives and processes OSC events.
	 * The receiver manages it's own connection in a background thread.
	 * All received messages are consumed by the nap::OSCService and dispatched on the main thread 
	 * to a valid nap::OSCInputComponent. Listen to the messageReceived signal of the OSC input component 
	 * to receive OSC events in the running application.
	 */
	class NAPAPI OSCReceiver : public Device
	{
		friend class OSCService;
		RTTI_ENABLE(Device)
	public:
		// Constructor used by factory
		OSCReceiver(OSCService& service);

		/**
		 * Initializes the receiver and registers it with the OSCService
		 * This call will also start the background thread that received the messages
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stop the OSCReceiver
		 */
		virtual void stop() override;

	public:
		int mPort = 7000;				///< Property: 'Port' The port that is opened and used to receive osc messages
		bool mDebugOutput = false;		///< Property: 'EnableDebugOutput' when enabled this objects prints all received osc messages
		bool mAllowPortReuse = false;	///< Property: 'AllowPortReuse' enables / disables multiple listeners for a single port on the same network interface

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