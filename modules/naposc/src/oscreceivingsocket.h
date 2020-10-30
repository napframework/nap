/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <ip/UdpSocket.h>
#include <osc/OscPacketListener.h>

namespace nap
{
	/**
	 *	Socket used for receiving OSC messages over the network
	 */
	class OSCReceivingSocket : public UdpSocket
	{
	public:
		/**
		 * This constructor allows the socket to receive OSC messages from any computer in the network
		 * @param port the port to listen to for OSC messages
		 * @param allowReuse enables or disables multiple listeners for a single port on the same network interface
		 */
		OSCReceivingSocket(int port, bool allowReuse);

		/**
		 * Constructor
		 * @param localEndpoint network location to listen for messages
		 * @param allowReuse enables or disables multiple listeners for a single port on the same network interface
		 */
		OSCReceivingSocket(const IpEndpointName& localEndpoint, bool allowReuse);

		/**
		 *	Unbinds the listener
		 */
		~OSCReceivingSocket();

		/**
		 * Sets the listener that is used to forward the incoming packages to
		 * @param listener the listener to send the packages to
		 */
		void setListener(osc::OscPacketListener* listener);
		
		/**
		 *	Start listening to messages until interrupted
		 */
		void run();

		/**
		 * Signals the handler to exit the run state
		 */
		void stop();

	private:
		SocketReceiveMultiplexer			mMultiplexer;				// Holds the port and allows for setting the listener
		PacketListener*						mListener = nullptr;		// The listener that handles the incoming OSC messages
	};
}
