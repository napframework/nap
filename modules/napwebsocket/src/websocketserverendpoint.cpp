// Local Includes
#include "websocketserverendpoint.h"

// External Includes
#include <nap/logger.h>

namespace nap
{
	WebSocketServerEndPoint::WebSocketServerEndPoint(int port, uint32 logLevel, uint32 accessLevel) :
		mPort(port), mLogLevel(logLevel), mAccessLogLevel(accessLevel)
	{
		// Initialize asio
		mEndPoint.init_asio();
	}



	bool WebSocketServerEndPoint::open(nap::utility::ErrorState& error)
	{
		// Run until stopped
		assert(!mOpen);
		
		// Initiate logging
		mEndPoint.clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint.set_error_channels(mLogLevel);

		mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
		mEndPoint.set_access_channels(mAccessLogLevel);

		// Install connection open / closed handlers
		mEndPoint.set_open_handler(std::bind(&WebSocketServerEndPoint::connectionOpened,  this, std::placeholders::_1));
		mEndPoint.set_close_handler(std::bind(&WebSocketServerEndPoint::connectionClosed, this, std::placeholders::_1));
		mEndPoint.set_fail_handler(std::bind(&WebSocketServerEndPoint::connectionFailed,  this, std::placeholders::_1));

		// Listen to messages on this specific port
		std::error_code stdec;
		mEndPoint.listen(static_cast<uint16>(mPort), stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Queues a connection accept operation 
		mEndPoint.start_accept(stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Run until stopped
		mOpen = true;
		mServerTask = std::async(std::launch::async, std::bind(&WebSocketServerEndPoint::run, this));

		return true;
	}


	void WebSocketServerEndPoint::send(const std::string& message, wspp::Connection connection, wspp::OpCode opCode)
	{
		mEndPoint.send(connection, message, opCode);
	}


	void WebSocketServerEndPoint::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	void WebSocketServerEndPoint::close()
	{
		if (mOpen)
		{
			assert(mServerTask.valid());
			mEndPoint.stop();
			mServerTask.wait();
			mOpen = false;
		}
	}


	WebSocketServerEndPoint::~WebSocketServerEndPoint()
	{
		close();
	}


	void WebSocketServerEndPoint::setMessageHandler(wspp::MessageHandler message_handler)
	{
		mEndPoint.set_message_handler(message_handler);
	}
	

	void WebSocketServerEndPoint::connectionOpened(wspp::Connection connection)
	{
		nap::Logger::info("Connection opened!");
	}


	void WebSocketServerEndPoint::connectionClosed(wspp::Connection connection)
	{
		nap::Logger::info("Connection closed!");
	}


	void WebSocketServerEndPoint::connectionFailed(wspp::Connection connection)
	{
		nap::Logger::info("Connection failed!");
	}

}
