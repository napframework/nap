#include "websocketclientendpoint.h"
#include "websocketclient.h"
#include "websocketmessage.h"

// External Includes
#include <nap/logger.h>

// nap::websocketclientendpoint run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketClientEndPoint)
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::WebSocketClientEndPoint::mLogConnectionUpdates,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel",		&nap::WebSocketClientEndPoint::mLibraryLogLevel,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{	
	bool WebSocketClientEndPoint::init(utility::ErrorState& errorState)
	{
		// Convert log levels
		mLogLevel = computeWebSocketLogLevel(mLibraryLogLevel);
		mAccessLogLevel = mLogConnectionUpdates ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
			: websocketpp::log::alevel::fail;

		// Initiate logging
		mEndPoint.clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint.set_error_channels(mLogLevel);

		mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
		mEndPoint.set_access_channels(mAccessLogLevel);

		// Init asio
		std::error_code stdec;
		mEndPoint.init_asio(stdec);
		if (stdec)
		{
			errorState.fail(stdec.message());
			return false;
		}

		return true;
	}


	void WebSocketClientEndPoint::stop()
	{		
		// At this state we need to have an open end point and client thread
		assert(mRunning);
		assert(mClientTask.valid());

		// Make sure the end point doesn't restart
		mEndPoint.stop_perpetual();

		// Wait until all still active clients exit clean
		for (auto& client : mClients)
		{
			utility::ErrorState error;
			if (!client->disconnect(error))
				nap::Logger::error(error.toString());
		}

		// Wait until all clients exited
		mClientTask.wait();
		mRunning = false;

		// Clear all clients
		mClients.clear();
	}


	bool WebSocketClientEndPoint::start(utility::ErrorState& error)
	{		
		// Ensure state
		assert(!mRunning);

		// Ensure connection exists when server disconnects
		mEndPoint.start_perpetual();

		// Run client in background
		mClientTask = std::async(std::launch::async, std::bind(&WebSocketClientEndPoint::run, this));

		mRunning = true;

		return true;
	}


	bool WebSocketClientEndPoint::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		std::error_code stdec;
		mEndPoint.send(connection.mConnection, message, static_cast<wspp::OpCode>(code), stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}
		return true;
	}
	

	bool WebSocketClientEndPoint::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
	{
		std::error_code stdec;
		mEndPoint.send(connection.mConnection, payload, length, static_cast<wspp::OpCode>(code), stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}
		return true;
	}


	void WebSocketClientEndPoint::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	bool WebSocketClientEndPoint::registerClient(IWebSocketClient& client, utility::ErrorState& error)
	{
		// Get shared pointer to connection
		std::error_code stdec;
		wspp::ConnectionPtr client_connection = mEndPoint.get_connection(client.mURI, stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Send identification information if present on client
		if (client.mTicket != nullptr)
		{
			// Convert ticket into binary string
			std::string binary_string;
			if (!client.mTicket->toBinaryString(binary_string, error))
				return false;

			// Add as sub-protocol identifier.
			client_connection->add_subprotocol(binary_string, stdec);
			if (stdec)
			{
				error.fail(stdec.message());
				return false;
			}
		}

		// Create meta client and add to the list of internally managed clients
		std::unique_ptr<WebSocketClientWrapper> meta_client(new WebSocketClientWrapper(client,
			mEndPoint,
			client_connection));
		mClients.emplace_back(std::move(meta_client));

		// Store connection handle in client
		client.mConnection = WebSocketConnection(client_connection->get_handle());

		// TRY to connect, this occurs on a background thread.
		// All connection related state changes are handled in the WebSocketClientWrapper.
		mEndPoint.connect(client_connection);
	
		return true;
	}


	void WebSocketClientEndPoint::unregisterClient(const IWebSocketClient& client)
	{
		auto found_it = std::find_if(mClients.begin(), mClients.end(), [&](const auto& it)
		{
			return it->mResource == &client;
		});

		if (found_it == mClients.end())
		{
			return;
		}

		// Disconnect if connected previously
		utility::ErrorState error;
		if (!(*found_it)->disconnect(error))
		{
			nap::Logger::error(error.toString());
		}
		mClients.erase(found_it);
	}


	WebSocketClientWrapper::WebSocketClientWrapper(IWebSocketClient& client, wspp::ClientEndPoint& endPoint, wspp::ConnectionPtr connection) :
		mResource(&client), mEndPoint(&endPoint), mHandle(connection->get_handle())
	{
		// Connect callbacks (occur on different thread)
		connection->set_open_handler(std::bind(&WebSocketClientWrapper::onConnectionOpened, this, std::placeholders::_1));
		connection->set_close_handler(std::bind(&WebSocketClientWrapper::onConnectionClosed, this, std::placeholders::_1));
		connection->set_fail_handler(std::bind(&WebSocketClientWrapper::onConnectionFailed, this, std::placeholders::_1));

		// Install message handler
		connection->set_message_handler(std::bind(
			&WebSocketClientWrapper::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2));
	}


	WebSocketClientWrapper::~WebSocketClientWrapper()
	{
		// Not disconnected by server or client
		assert(!mOpen);
		mResource = nullptr;
		mEndPoint = nullptr;
	}


	void WebSocketClientWrapper::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		assert(mResource != nullptr);
		mResource->connectionOpened();
		mOpen = true;
	}


	void WebSocketClientWrapper::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		assert(mResource != nullptr);
		mResource->connectionClosed(stdec.value(), stdec.message());
		mOpen = false;
	}


	void WebSocketClientWrapper::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		assert(mResource != nullptr);
		mResource->connectionFailed(stdec.value(), stdec.message());
		mOpen = false;
	}


	void WebSocketClientWrapper::onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg)
	{
		assert(mResource != nullptr);
		mResource->messageReceived(WebSocketMessage(msg));
	}


	bool WebSocketClientWrapper::disconnect(nap::utility::ErrorState& error)
	{
		if (mOpen)
		{
			// Get connection from handle, should be open by this point
			std::error_code stdec;
			wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(mHandle, stdec);
			if (stdec)
			{
				assert(false);
				error.fail(stdec.message());
				return false;
			}

			// Remove callbacks
			cptr->set_open_handler(nullptr);
			cptr->set_close_handler(nullptr);
			cptr->set_fail_handler(nullptr);
			cptr->set_message_handler(nullptr);
			
			// Now close
			mEndPoint->close(mHandle, websocketpp::close::status::going_away, "disconnected", stdec);
			if (stdec)
			{
				nap::Logger::error(stdec.message());
			}
			mOpen = false;
		}
		return true;
	}

}