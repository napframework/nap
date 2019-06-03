// Local Includes
#include "websocketserverendpoint.h"
#include "websocketserver.h"

RTTI_BEGIN_CLASS(nap::WebSocketServerEndPoint)
	RTTI_PROPERTY("Port",					&nap::WebSocketServerEndPoint::mPort,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::WebSocketServerEndPoint::mLogConnectionUpdates,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel",		&nap::WebSocketServerEndPoint::mLibraryLogLevel,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	WebSocketServerEndPoint::WebSocketServerEndPoint()
	{

	}

	bool WebSocketServerEndPoint::start(nap::utility::ErrorState& error)
	{
		// Run until stopped
		assert(!mOpen);
		
		// Initiate logging
		mEndPoint.clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint.set_error_channels(mLogLevel);

		mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
		mEndPoint.set_access_channels(mAccessLogLevel);

		// Install connection open / closed handlers
		mEndPoint.set_open_handler(std::bind(&WebSocketServerEndPoint::onConnectionOpened,	this, std::placeholders::_1));
		mEndPoint.set_close_handler(std::bind(&WebSocketServerEndPoint::onConnectionClosed,	this, std::placeholders::_1));
		mEndPoint.set_fail_handler(std::bind(&WebSocketServerEndPoint::onConnectionFailed,	this, std::placeholders::_1));
		
		// Install message handler
		mEndPoint.set_message_handler(std::bind(
			&WebSocketServerEndPoint::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2
		));

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


	void WebSocketServerEndPoint::send(const std::string& message, wspp::ConnectionHandle connection, wspp::OpCode opCode)
	{
		mEndPoint.send(connection, message, opCode);
	}


	void WebSocketServerEndPoint::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	void WebSocketServerEndPoint::stop()
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
		stop();
	}


	bool WebSocketServerEndPoint::init(utility::ErrorState& errorState)
	{
		// Convert log levels
		mLogLevel = computeWebSocketLogLevel(mLibraryLogLevel);
		mAccessLogLevel = mLogConnectionUpdates ? websocketpp::log::alevel::all ^ websocketpp::log::alevel::frame_payload
			: websocketpp::log::alevel::fail;

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
	

	void WebSocketServerEndPoint::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		for (auto& listener : mListeners)
		{
			if (listener->accepts(RTTI_OF(WebSocketConnectionOpenedEvent)))
			{
				listener->addEvent(std::make_unique<WebSocketConnectionOpenedEvent>(WebSocketConnection(connection)));
			}
		}
	}


	void WebSocketServerEndPoint::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		for (auto& listener : mListeners)
		{
			if (listener->accepts(RTTI_OF(WebSocketConnectionClosedEvent)))
			{
				listener->addEvent(std::make_unique<WebSocketConnectionClosedEvent>(WebSocketConnection(connection)));
			}
		}
	}


	void WebSocketServerEndPoint::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		for (auto& listener : mListeners)
		{
			if (listener->accepts(RTTI_OF(WebSocketConnectionFailedEvent)))
			{
				listener->addEvent(std::make_unique<WebSocketConnectionFailedEvent>(WebSocketConnection(connection)));
			}
		}
	}


	void WebSocketServerEndPoint::onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg)
	{
		for (auto& listener : mListeners)
		{
			if (listener->accepts(RTTI_OF(WebSocketMessageReceivedEvent)))
			{
				listener->addEvent(std::make_unique<WebSocketMessageReceivedEvent>(WebSocketConnection(con), WebSocketMessage(msg)));
			}
		}
		send("who's your daddy now??", con, msg->get_opcode());
	}


	void WebSocketServerEndPoint::registerListener(IWebSocketServer& listener)
	{
		mListeners.emplace_back(&listener);
	}


	void WebSocketServerEndPoint::removeListener(IWebSocketServer& listener)
	{
		auto found_it = std::find_if(mListeners.begin(), mListeners.end(), [&](const auto& it)
		{
			return it == &listener;
		});
		assert(found_it != mListeners.end());
		mListeners .erase(found_it);
	}
}
