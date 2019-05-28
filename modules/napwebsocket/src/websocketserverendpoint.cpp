// Local Includes
#include "websocketserverendpoint.h"

namespace nap
{
	WebSocketServerEndPoint::WebSocketServerEndPoint(int port, uint32 logLevel, bool logConnectionAccess) :
		mPort(port), mLogLevel(logLevel), mLogConnectionAccess(logConnectionAccess)
	{
		// Initialize asio
		mEndPoint.init_asio();
	}


	void WebSocketServerEndPoint::open()
	{
		// Run until stopped
		mServerTask = std::async(std::launch::async, std::bind(&WebSocketServerEndPoint::run, this));
	}


	void WebSocketServerEndPoint::send(const std::string& message, websocketpp::connection_hdl connection, PPServerEndPoint::message_ptr originalMessage)
	{
		mEndPoint.send(connection, message, originalMessage->get_opcode());
	}


	void WebSocketServerEndPoint::run()
	{
		// Initiate logging
		mEndPoint.clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint.set_error_channels(mLogLevel);

		mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
		uint32 alog_level = mLogConnectionAccess ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
			: websocketpp::log::alevel::fail;
		mEndPoint.set_access_channels(alog_level);

		// Listen to messages on this specific port
		mEndPoint.listen(mPort);

		// Queues a connection accept operation 
		mEndPoint.start_accept();

		// Start running ontil stopped
		mEndPoint.run();
	}


	void WebSocketServerEndPoint::close()
	{
		if (mServerTask.valid())
		{
			mEndPoint.stop();
			mServerTask.wait();
		}
	}


	WebSocketServerEndPoint::~WebSocketServerEndPoint()
	{
		close();
	}


	void WebSocketServerEndPoint::setHandler(std::function<void(websocketpp::connection_hdl, PPServerEndPoint::message_ptr)> message_handler)
	{
		mEndPoint.set_message_handler(message_handler);
	}
}
