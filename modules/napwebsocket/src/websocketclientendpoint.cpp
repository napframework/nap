#include "websocketclientendpoint.h"
#include "websocketclient.h"

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
			assert(mEndPoint != nullptr);
			assert(mClientTask.valid());

			// Make sure the end point doesn't restart
			mEndPoint.stop_perpetual();

			// Notify server we're disconnecting, connection is closed if ever established
			utility::ErrorState napec;
			if (!close(napec))
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


	void WebSocketClientEndPoint::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	bool WebSocketClientEndPoint::connect(IWebSocketClient& client, utility::ErrorState& error)
	{
		assert(mEndPoint != nullptr);

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
		
		// Add it as a valid connection
		{
			std::lock_guard<std::mutex> guard(mConnectionMutex);
			mConnections.emplace_back(client_connection);
		}
		
		// Try to connect
		mEndPoint.connect(client_connection);

		// Set the handle
		client.mConnection = WebSocketConnection(client_connection->get_handle());

		// Connect to destruction slot
		client.destroyed.connect(mClientDestroyed);

		return true;
	}


	bool WebSocketClientEndPoint::close(utility::ErrorState& error)
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		bool success = true;
		for (auto& connection : mConnections)
		{
			std::error_code stdec;
			mEndPoint.close(connection, websocketpp::close::status::going_away, "disconnected", stdec);
			if (stdec)
			{
				error.fail(stdec.message());
				success = false;
			}
		}
		mConnections.clear();
		return success;
	}


	void WebSocketClientEndPoint::onClientDestroyed(const WebSocketConnection& connection)
	{
		// Remove from internal list of connections
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection.mConnection, stdec);
		if (stdec)
		{
			assert(false);
			nap::Logger::error(stdec.message());
			return;
		}

		{
			std::lock_guard<std::mutex> lock(mConnectionMutex);
			auto found_it = std::find_if(mConnections.begin(), mConnections.end(), [&](const auto& it)
			{
				return it == cptr;
			});

			if (found_it == mConnections.end())
			{
				assert(false);
				return;
			}
			mConnections.erase(found_it);
		}

		// Close it if it is open
		if (cptr->get_state() == websocketpp::session::state::open)
		{
			mEndPoint.close(connection.mConnection, websocketpp::close::status::going_away, "disconnected", stdec);
			if (stdec)
			{
				assert(false);
				nap::Logger::error("%s: %s", this->mID.c_str(), stdec.message().c_str());
			}
		}
	}
}