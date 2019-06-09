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
	WebSocketClientEndPoint::~WebSocketClientEndPoint()
	{
		stop();
	}

	
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
		if (mRunning)
		{
			// At this state we need to have an open end point and client thread
			assert(mClientTask.valid());

			// Make sure the end point doesn't restart
			mEndPoint.stop_perpetual();

			// Notify server we're disconnecting, connection is closed if ever established
			utility::ErrorState napec;
			if (!closeAll(napec))
			{
				//assert(false);
				nap::Logger::error("%s: %s", this->mID.c_str(), napec.toString().c_str());
			}

			mClientTask.wait();
			mRunning = false;
		}
	}


	bool WebSocketClientEndPoint::start(utility::ErrorState& error)
	{		
		// Run until stopped
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
		// Add it as a valid connection
		{
			std::lock_guard<std::mutex> guard(mConnectionMutex);
			mClients.emplace_back(&client);
		}

		// Get shared pointer to connection
		std::error_code stdec;
		wspp::ConnectionPtr client_connection = mEndPoint.get_connection(client.mURI, stdec);
		
		// If an error occured return an invalid web-socket connection
		if (stdec)
		{
			error.fail(stdec.message());
			client.mConnection = WebSocketConnection();
			return false;
		}

		client_connection->set_open_handler(std::bind(&WebSocketClientEndPoint::onConnectionOpened, this, std::placeholders::_1));
		client_connection->set_close_handler(std::bind(&WebSocketClientEndPoint::onConnectionClosed, this, std::placeholders::_1));
		client_connection->set_fail_handler(std::bind(&WebSocketClientEndPoint::onConnectionFailed, this, std::placeholders::_1));

		// Install message handler
		client_connection->set_message_handler(std::bind(
			&WebSocketClientEndPoint::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2));

		// Try to connect
		mEndPoint.connect(client_connection);

		// Set the handle
		client.mConnection = WebSocketConnection(client_connection->get_handle());

		// Connect to client disconnect slot
		client.destroyed.connect(mClientDestroyed);

		return true;
	}


	bool WebSocketClientEndPoint::closeAll(utility::ErrorState& error)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		bool success = true;
		for (auto& client : mClients)
		{
			if(!client->isOpen())
				continue;

			std::error_code stdec;
			mEndPoint.close(client->mConnection.mConnection, websocketpp::close::status::going_away, "disconnected", stdec);
			if (stdec)
			{
				error.fail(stdec.message());
				success = false;
			}
		}
		mClients.clear();
		return success;
	}


	void WebSocketClientEndPoint::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		assert(!stdec);

		// Find matching connection
		IWebSocketClient* found_client = findClient(cptr);
		
		// The client can be null when close() has been called or the client has been destructed already
		// The mutex ensures that order of removal / addition is secure.
		if (found_client != nullptr)
		{
			found_client->connectionOpened();
		}

	}


	void WebSocketClientEndPoint::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		assert(!stdec);

		// Find matching connection
		IWebSocketClient* found_client = findClient(cptr);

		// The client can be null when close() has been called or the client has been destructed already
		// The mutex ensures that order of removal / addition is secure.
		if (found_client != nullptr)
		{
			found_client->connectionClosed(cptr->get_ec().value(), cptr->get_ec().message());
		}
	}


	void WebSocketClientEndPoint::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		assert(!stdec);

		// Find matching connection
		IWebSocketClient* found_client = findClient(cptr);
		
		// The client can be null when close() has been called or the client has been destructed already
		// The mutex ensures that order of removal / addition is secure.
		if (found_client != nullptr)
		{
			found_client->connectionFailed(cptr->get_ec().value(), cptr->get_ec().message());
		}
	}


	void WebSocketClientEndPoint::onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		assert(!stdec);

		// Find matching connection
		IWebSocketClient* found_client = findClient(cptr);
		if (found_client != nullptr)
		{
			found_client->messageReceived(WebSocketMessage(msg));
		}
	}


	void WebSocketClientEndPoint::onClientDestroyed(const IWebSocketClient& client)
	{
		// Remove client from internally managed list
		removeClient(client);

		// Disconnect if current connection is open
		std::error_code stdec;
		if (client.isOpen())
		{
			mEndPoint.close(client.mConnection.mConnection, websocketpp::close::status::going_away, "disconnected", stdec);
			nap::Logger::error("%s: %s", this->mID.c_str(), stdec.message().c_str());
		}
	}


	nap::IWebSocketClient* WebSocketClientEndPoint::findClient(wspp::ConnectionPtr connection)
	{
		assert(connection != nullptr);
		std::error_code stdec;
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		for (auto client : mClients)
		{
			if (client->mConnection.expired())
				continue;

			wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(client->mConnection.mConnection, stdec);
			assert(!stdec);

			if (cptr == connection)
				return client;
		}
		return nullptr;
	}


	void WebSocketClientEndPoint::removeClient(const IWebSocketClient& client)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		auto found_it = std::find_if(mClients.begin(), mClients.end(), [&](const auto& it)
		{
			return it == &client;
		});

		if (found_it == mClients.end())
		{
			assert(false);
			return;
		}
		mClients.erase(found_it);
	}

}