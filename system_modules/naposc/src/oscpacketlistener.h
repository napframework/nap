/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "oscevent.h"

// External Includes
#include <osc/OscPacketListener.h>
#include <queue>
#include <mutex>

namespace nap
{
	class OSCReceiver;

	/**
	 *	Listens to incoming packages and translates those in to OSC events
	 */
	class NAPAPI OSCPacketListener : public osc::OscPacketListener
	{
		friend class OSCReceiver;
	public:
		OSCPacketListener(OSCReceiver& receiver);

		/**
		 *	@param value when set to true this listener will print all received messages
		 */
		void setDebugOutput(bool value)							{ mDebugOutput = value; }

	protected:
		virtual void ProcessMessage(const osc::ReceivedMessage& message, const IpEndpointName& remoteEndpoint) override;

	private:
		OSCReceiver& mReceiver;				// Receiver that holds the message queue
		bool mDebugOutput = false;			// When set to true the listener will print all the received messages

		/**
		 * Utility that prints @event
		 * @param event osc event to be debug printed
		 */
		static void displayMessage(const OSCEvent& event);
	};
}
