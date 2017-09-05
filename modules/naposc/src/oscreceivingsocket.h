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
		 */
		OSCReceivingSocket(int port);

		/**
		 * Constructor
		 * @param network location to listen to for messages
		 */
		OSCReceivingSocket(const IpEndpointName& localEndpoint);

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
		void asynchronousBreak();

	private:
		SocketReceiveMultiplexer			mMultiplexer;
		PacketListener*						mListener = nullptr;
	};
}
