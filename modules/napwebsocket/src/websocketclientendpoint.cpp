#include "websocketclientendpoint.h"

// External Includes
#include <nap/logger.h>

// nap::websocketclientendpoint run time class definition 
RTTI_BEGIN_CLASS(nap::WebSocketClientEndPoint)
	RTTI_PROPERTY("URI",					&nap::WebSocketClientEndPoint::mURI,					nap::rtti::EPropertyMetaData::Required)
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
			if (!disconnect(napec))
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

		// Get shared pointer to connection
		std::error_code stdec;
		mConnection = mEndPoint.get_connection(mURI, stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Run client in background
		mClientTask = std::async(std::launch::async, std::bind(&WebSocketClientEndPoint::run, this));

		// Connect
		connect();

		mRunning = true;

		return true;
	}


	bool WebSocketClientEndPoint::reconnect(utility::ErrorState& error)
	{
		return true;
	}


	void WebSocketClientEndPoint::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	void WebSocketClientEndPoint::connect()
	{
		assert(mEndPoint != nullptr);
		assert(mConnection != nullptr);
		mEndPoint.connect(mConnection);
	}


	bool WebSocketClientEndPoint::disconnect(nap::utility::ErrorState& error)
	{
		// TODO: Check client / server connection status
		std::error_code stdec;
		mEndPoint.close(mConnection->get_handle(), websocketpp::close::status::going_away, "disconnected", stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}
		return true;
	}

}