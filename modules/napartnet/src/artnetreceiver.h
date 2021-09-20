/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "artnetlistener.h"
#include "artnetevent.h"

// External Includes
#include <nap/device.h>
#include <rtti/factory.h>
#include <asio/io_service.hpp>
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
		* Adds an event to the queue
		* @param event the event to add, note that this receiver will take ownership of the event
		*/
		void addEvent(ArtNetEventPtr event);

		/**
		* Consumes all received Art-Net events and moves them to outEvents, called by the service on the main thread
		* Calling this will clear the internal queue and transfers ownership of the events to the caller
		* @param outEvents will hold the transferred Art-Net events
		*/
		void consumeEvents(std::queue<ArtNetEventPtr>& outEvents);

		std::string mIpAddress = "";	///< Property: 'IP Address' The IP address of the interface to use for receiving Art-Net, will use 0.0.0.0 (all local addresses) if left empty
		uint16 mPort = 6454;			///< Property: 'Port' The port that is opened and used to receive Art-Net messages

	private:
		friend class ArtNetService;

		// Art-Net service that consumes the events
		ArtNetService* mService = nullptr;

		// The ASIO I/O service
		asio::io_service mIOService;

		// The thread used for running the I/O service
		std::thread mRunThread;

		// The UDP socket server used for receiving Art-Net
		std::unique_ptr<ArtNetListener> mListener = nullptr;

		// Queue that holds all the consumed events
		std::queue<ArtNetEventPtr> mEvents;

		// Mutex associated with setting / getting events
		std::mutex mEventMutex;
	};

	using ArtNetReceiverCreator = rtti::ObjectCreator<ArtNetReceiver, ArtNetService>;
}
