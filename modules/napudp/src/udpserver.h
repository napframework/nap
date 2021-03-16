/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <nap/device.h>
#include <thread>
#include <mutex>

// NAP includes
#include <nap/numeric.h>
#include <concurrentqueue.h>

// ASIO includes
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/io_service.hpp>
#include <asio/system_error.hpp>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class UdpServerListener;

	/**
	 * The UdpServer connects to an endpoint and receives any UDP packets send to the endpoint
	 * The server will then forward the UDP packets to any registered listener that extend on UdpServerListener base class
	 * The UdpServer fires up its own thread and all UdpPackets pushed to listeners will happen on that thread
	 * Whenever a listeners is registered or removed it will happen thread-safe
	 */
	class UdpServer : public Device
	{
		RTTI_ENABLE(Device)
	public:
		/**
		 * Fires up the server thread and starts receiving udp
		 */
		virtual bool start(utility::ErrorState& errorState) override;

		/**
		 * Stops server thread and stop receiving udp
		 */
		virtual void stop() override;

		/**
		 * Registers a listener. This task will be queued to happen on server thread
		 * The listener will receive UdpPackets on the server thread
		 * The user is responsible for safely managing the listener and make sure it is not deconstructed while still
		 * registered to the server
		 * @param listener instance of the class that extends on UdpServerListener
		 */
		void registerListener(UdpServerListener * listener);

		/**
		 * Removes a listener. This task will be queue to happen on server thread
		 * @param listener instance of the class that extends on UdpServerListener
		 */
		void removeListener(UdpServerListener * listener);

		/**
		 * Enqueues a task and makes sure it is excecuted on server thread
		 * @param task the task to be excecuted
		 */
		void enqueueTask(std::function<void()> task);
	public:
		int mPort 						= 13251;		///< Property: 'Port' the port the server socket binds to
		std::string mIPRemoteEndpoint 	= "127.0.0.1";  ///< Property: 'Endpoint' the ip adress the server socket binds to
		int mBufferSize 				= 1024;			///< Property: 'BufferSize' the size of the buffer the server writes to
		bool mThrowOnInitError 			= true;			///< Property: 'ThrowOnFailure' when server fails to bind socket, return false on start
	private:
		/**
		 * The threaded function
		 */
		void receiveThread();

		/**
		 * The callback called upon receiving udp
		 * @param error the error_code on error
		 * @param bytesTransferred the amount of bytes transferred
		 */
		void handleReceive(const asio::error_code& error, size_t bytesTransferred);

		// ASIO
		asio::io_service 			mIOService;
		asio::ip::udp::socket 		mSocket{mIOService};
		std::vector<nap::int8>		mBuffer;
		asio::ip::udp::endpoint 	mRemoteEndpoint;

		// Threading
		std::thread 										mReceiverThread;
		std::atomic_bool 									mRun;
		moodycamel::ConcurrentQueue<std::function<void()>> 	mTaskQueue;

		// Listeners
		std::vector<UdpServerListener*> 	mListeners;
	};
}
