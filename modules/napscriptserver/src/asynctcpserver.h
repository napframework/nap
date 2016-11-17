#pragma once

#include <iostream>
#include <map>
#include <nap/logger.h>
#include <nap/signalslot.h>
#include <queue>
#include <thread>
#include <time.h>
#include <zmq.hpp>

namespace nap
{

    class AsyncTCPServer;

	/**
	 * Keeps track of a client connected to a AsyncTCPServer and holds a bunch of data relevant to that client.
	 */
	class AsyncTCPClient
	{
        friend class AsyncTCPServer;
    private:
        // Creates a client with the specified identity, may only be constructed by the AsyncTCPServer
		AsyncTCPClient(const std::string& ident) : mIdent(ident) {}
    public:
		// Return the last time this client was seen alive
		time_t getLastHeartbeat() const { return mLastHeartBeat; }

		// Keep this client alive
		void updateHeartbeat() { mLastHeartBeat = time(nullptr); }

		// Returns the client's identifier
		const std::string& getIdent() const { return mIdent; }

		// Any events queued for this client?
		bool hasEvents() const { return !mEventQueue.empty(); }

		// Add an event to push to this client
		void enqueueEvent(const std::string& msg) { mEventQueue.push(msg); }

		// Grab and remove the next event for this client
		std::string popEvent();

		// Triggered when the client has sent a message
		Signal<const std::string&> messageReceived;

		// Triggered when this client has given up
		Signal<AsyncTCPClient&> disconnected;

	private:
		std::queue<std::string> mEventQueue;
		const std::string mIdent;
		time_t mLastHeartBeat;
	};



	/**
	 * Asynchronous server that allows multiple clients to connect to a single port.
	 * Clients can send messages to the server and may asynchronously receive events/messages at any time.
	 *
	 * The server uses AsyncTCPClient instances to keep track of connected clients.
	 *
	 * Also, this server is agnostic to any data passed around, apart from an identity, the user must implement the
	 * protocol.
	 */
	class AsyncTCPServer
	{
	public:
		// Create a threaded server that will immediately start listening on the specified port
		AsyncTCPServer(int port);

		// Triggered when a client has connected
		Signal<AsyncTCPClient&> clientConnected;

		// Triggered when a client has disconnected
		Signal<AsyncTCPClient&> clientDisconnected;

		// Triggered when a client has sent a message to this server
		Signal<AsyncTCPClient&, const std::string&> requestReceived;

        // Get the client with specified ident
        AsyncTCPClient* getClient(const std::string& ident);

        // Break the server loop and attempt to exit cleanly
		void exit() { mRunning = false; }

	private:
		// Check each client's last heartbeat and discard when timed out
		void validateClients();

		// Retrieve client with specified identity and update heartbeat
		AsyncTCPClient* getOrAddClient(const std::string& ident);

		// Will be run on separate thread
		void runServer();

		// Send a multipart message to the client with specified identity
		void sendMultipart(zmq::socket_t& sock, const std::string& ident, const std::string& data);

	private:
		time_t mClientTimeout = 3;
		bool mRunning = true;
		int mPort;
		std::map<std::string, std::unique_ptr<AsyncTCPClient>> mClients;
		std::unique_ptr<std::thread> mThread;
	};
}