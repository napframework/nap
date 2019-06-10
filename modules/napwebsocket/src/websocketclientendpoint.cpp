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

			// Wait until all clients exited clean
			mClients.clear();
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

		// Create meta client
		std::unique_ptr<WebSocketMetaClient> meta_client = std::make_unique<WebSocketMetaClient>(client, 
			mEndPoint, 
			client_connection->get_handle());

		// Connect callbacks (occur on different thread)
		client_connection->set_open_handler(std::bind(&WebSocketMetaClient::onConnectionOpened, meta_client.get(), std::placeholders::_1));
		client_connection->set_close_handler(std::bind(&WebSocketMetaClient::onConnectionClosed, meta_client.get(), std::placeholders::_1));
		client_connection->set_fail_handler(std::bind(&WebSocketMetaClient::onConnectionFailed, meta_client.get(), std::placeholders::_1));

		// Install message handler
		client_connection->set_message_handler(std::bind(
			&WebSocketMetaClient::onMessageReceived, meta_client.get(),
			std::placeholders::_1, std::placeholders::_2));

		// Try to connect
		mEndPoint.connect(client_connection);
		 
		// Set the handle in the resource
		client.mConnection = WebSocketConnection(client_connection->get_handle());
		mClients.emplace_back(std::move(meta_client));

		// Connect to client disconnect slot
		client.destroyed.connect(mClientDestroyed);

		return true;
	}


	void WebSocketClientEndPoint::onClientDestroyed(const IWebSocketClient& client)
	{
		// Remove client from internally managed list
		removeClient(client);
	}


	void WebSocketClientEndPoint::removeClient(const IWebSocketClient& client)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		auto found_it = std::find_if(mClients.begin(), mClients.end(), [&](const auto& it)
		{
			return it->mResource == &client;
		});

		if (found_it == mClients.end())
		{
			assert(false);
			return;
		}
		mClients.erase(found_it);
	}


	WebSocketMetaClient::WebSocketMetaClient(IWebSocketClient& client, wspp::ClientEndPoint& endPoint, wspp::ConnectionPtr connection) :
		mResource(&client), mEndPoint(&endPoint), mHandle(connection->get_handle())
	{
		// Connect callbacks (occur on different thread)
		connection->set_open_handler(std::bind(&WebSocketMetaClient::onConnectionOpened, this, std::placeholders::_1));
		connection->set_close_handler(std::bind(&WebSocketMetaClient::onConnectionClosed, this, std::placeholders::_1));
		connection->set_fail_handler(std::bind(&WebSocketMetaClient::onConnectionFailed, this, std::placeholders::_1));

		// Install message handler
		connection->set_message_handler(std::bind(
			&WebSocketMetaClient::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2));
	}


	WebSocketMetaClient::~WebSocketMetaClient()
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		if (mOpen)
		{
			// Get actual connetion from handle
			// Should be valid because the connection is flagged to be open
			std::error_code stdec;
			wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(mHandle, stdec);
			assert(!stdec);
			
			// Remove callbacks!
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
		}
		mResource = nullptr;
		mEndPoint = nullptr;
	}


	void WebSocketMetaClient::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		if (mResource != nullptr)
			mResource->connectionOpened();
		mOpen = true;
	}


	void WebSocketMetaClient::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		std::lock_guard<std::mutex> lock(mConnectionMutex);
		if (mResource != nullptr)
			mResource->connectionClosed(stdec.value(), stdec.message());
		mOpen = false;
	}


	void WebSocketMetaClient::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		// Extract actual connection, must be valid at this point
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection, stdec);
		assert(!stdec);

		std::lock_guard<std::mutex> lock(mConnectionMutex);
		if (mResource != nullptr)
			mResource->connectionFailed(stdec.value(), stdec.message());
		mOpen = false;
	}


	void WebSocketMetaClient::onMessageReceived(wspp::ConnectionHandle connection, wspp::MessagePtr msg)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		if(mResource != nullptr)
		{
			mResource->messageReceived(WebSocketMessage(msg));
		}
	}


	void WebSocketMetaClient::clearResource()
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		mResource = nullptr;
	}

}