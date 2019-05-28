// Local Includes
#include "websocketserverendpoint.h"

namespace nap
{
	WebSocketServerEndPoint::WebSocketServerEndPoint(int port, uint32 logLevel, uint32 accessLevel) :
		mPort(port), mLogLevel(logLevel), mAccessLogLevel(logConnectionAccess)
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


	void WebSocketServerEndPoint::send(const std::string& message, websocketpp::connection_hdl connection, PPServerEndPoint::message_ptr originalMessage)
	{
		mEndPoint.send(connection, message, originalMessage->get_opcode());
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


	void WebSocketServerEndPoint::setMessageHandler(std::function<void(websocketpp::connection_hdl, PPServerEndPoint::message_ptr)> message_handler)
	{
		mEndPoint.set_message_handler(message_handler);
	}
}
