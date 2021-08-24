/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "artnetlistener.h"
#include "artdmxpacketevent.h"

// External Includes
#include <nap/device.h>
#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include <cstdint>

namespace nap
{
	// Forward Declares
	class ArtNetService;

	/**
	 * Receives and processes Art-Net messages.
	 */
	class NAPAPI ArtNetReceiver : public Device
	{
		RTTI_ENABLE(Device)

	public:

		// Default constructor
		ArtNetReceiver() = default;

		// Constructor used by factory
		ArtNetReceiver(ArtNetService & service);

		/**
		* Initializes the Art-Net receiver.
		* @param errorState Contains error information in case the function returns false.
		* @return true on success, false otherwise. In case of an error, errorState contains error information.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Starts the Art-Net receiver.
		 * @param errorState Contains error information in case the function returns false.
		 * @return true on success, false otherwise. In case of an error, errorState contains error information.
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops the Art-Net receiver.
		 */
		virtual void stop() override;

		/**
		* Destroys the Art-Net receiver.
		*/
		virtual void onDestroy() override;

		/**
		* Adds an event to the queue
		* @param event the event to add, note that this receiver will take ownership of the event
		*/
		void addEvent(ArtDmxPacketEventPtr event);

		/**
		* Consumes all received Art-Net events and moves them to outEvents, called by the service on the main thread
		* Calling this will clear the internal queue and transfers ownership of the events to the caller
		* @param outEvents will hold the transferred Art-Net events
		*/
		void consumeEvents(std::queue<ArtDmxPacketEventPtr>& outEvents);

		uint16_t mPort = 6454;    ///< Property: 'Port' The port that is opened and used to receive Art-Net messages

	private:
		friend class ArtNetService;

		// Art-Net service that consumes the events
		ArtNetService* mService = nullptr;

		// The ASIO I/O service
		void* mIOServiceHandle = nullptr;

		// The thread used for running the I/O service
		std::thread mRunThread;

		// The UDP socket server used for receiving Art-Net
		std::unique_ptr<ArtNetListener> mListener = nullptr;

		// Queue that holds all the consumed events
		std::queue<ArtDmxPacketEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex mEventMutex;
	};

	using ArtNetReceiverCreator = rtti::ObjectCreator<ArtNetReceiver, ArtNetService>;
}
