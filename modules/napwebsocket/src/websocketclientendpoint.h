#pragma once

// Local Includes
#include "websocketutils.h"
#include "websocketconnection.h"
#include "websocketmessage.h"

// External Includes
#include <nap/device.h>
#include <memory.h>
#include <future>
#include <nap/signalslot.h>
#include <mutex>

namespace nap
{
	class IWebSocketClient;

	/**
	 * Web-socket client endpoint.
	 */
	class NAPAPI WebSocketClientEndPoint : public Device
	{
		friend class IWebSocketClient;
		RTTI_ENABLE(Device)
	public:
		virtual ~WebSocketClientEndPoint();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		virtual void stop() override;

		virtual bool start(nap::utility::ErrorState& error) override;

		bool mAllowFailure = true;			///< Property: 'AllowFailure' if the client connection to the server is allowed to fail on start

		nap::Signal<const WebSocketConnection&> connectionOpened;
		nap::Signal<const WebSocketConnection&, int, const std::string&> connectionClosed;
		nap::Signal<const WebSocketConnection&, int, const std::string&> connectionFailed;
		nap::Signal<const WebSocketConnection&, const WebSocketMessage&> messageReceived;

		bool mLogConnectionUpdates = true;										///< Property: "LogConnectionUpdates" if client / server connection information is logged to the console.
		EWebSocketLogLevel mLibraryLogLevel = EWebSocketLogLevel::Warning;		///< Property: "LibraryLogLevel" library related equal to or higher than requested are logged.

	private:
		uint32 mLogLevel = 0;													///< Converted library log level
		uint32 mAccessLogLevel = 0;												///< Log client / server connection data
		bool mRunning = false;													///< If the client connection to the server is open						
		wspp::ClientEndPoint mEndPoint;											///< Websocketpp client end point
		std::vector<wspp::ConnectionPtr> mConnections;							///< Websocketpp client connections
		std::future<void> mClientTask;											///< The client server thread
		std::mutex mConnectionMutex;											///< Ensures connectivity thread safety

		/**
		 * Runs the end point in a background thread until stopped.
		 */
		void run();

		/**
		 * Connects a client to a server. The connection is managed by this endpoint.
		 * @param uri server uri.
		 * @return the newly created web-socket connection
		 */
		bool connect(IWebSocketClient& client , utility::ErrorState& error);

		/**
		 * Closes all active connections
		 */
		bool close(utility::ErrorState& error);

		void onClientDestroyed(const WebSocketConnection& connection);
		nap::Slot<const WebSocketConnection&> mClientDestroyed = { this, &WebSocketClientEndPoint::onClientDestroyed };
	};
}
