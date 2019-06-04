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
		assert(mEndPoint == nullptr);
		mEndPoint = std::make_unique<wspp::ServerEndPoint>();

		// Initiate logging
		mEndPoint->clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint->set_error_channels(mLogLevel);

		mEndPoint->clear_access_channels(websocketpp::log::alevel::all);
		mEndPoint->set_access_channels(mAccessLogLevel);

		// Init asio
		std::error_code stdec;
		mEndPoint->init_asio(stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Install connection open / closed handlers
		mEndPoint->set_http_handler(std::bind(&WebSocketServerEndPoint::onHTTP, this, std::placeholders::_1));
		mEndPoint->set_open_handler(std::bind(&WebSocketServerEndPoint::onConnectionOpened,	this, std::placeholders::_1));
		mEndPoint->set_close_handler(std::bind(&WebSocketServerEndPoint::onConnectionClosed,	this, std::placeholders::_1));
		mEndPoint->set_fail_handler(std::bind(&WebSocketServerEndPoint::onConnectionFailed,	this, std::placeholders::_1));
		mEndPoint->set_validate_handler(std::bind(&WebSocketServerEndPoint::onValidate, this, std::placeholders::_1));

		// Install message handler
		mEndPoint->set_message_handler(std::bind(
			&WebSocketServerEndPoint::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2
		));

		// Listen to messages on this specific port
		mEndPoint->listen(static_cast<uint16>(mPort), stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Queues a connection accept operation 
		mEndPoint->start_accept(stdec);
		if (stdec)
		{
			error.fail(stdec.message());
			return false;
		}

		// Run until stopped
		mServerTask = std::async(std::launch::async, std::bind(&WebSocketServerEndPoint::run, this));

		return true;
	}


	bool WebSocketServerEndPoint::isOpen() const
	{
		return mEndPoint != nullptr;
	}


	void WebSocketServerEndPoint::send(const std::string& message, wspp::ConnectionHandle connection, wspp::OpCode opCode)
	{
		mEndPoint->send(connection, message, opCode);
	}


	void WebSocketServerEndPoint::run()
	{
		// Start running until stopped
		mEndPoint->run();
	}


	void WebSocketServerEndPoint::stop()
	{
		if (mEndPoint != nullptr)
		{
			assert(mServerTask.valid());
			mEndPoint->stop();
			mServerTask.wait();
			mEndPoint.reset(nullptr);
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

		return true;
	}
	

	void WebSocketServerEndPoint::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		connectionOpened(WebSocketConnection(connection));
	}


	void WebSocketServerEndPoint::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection);
		connectionClosed(WebSocketConnection(connection),
			cptr->get_ec().value(), 
			cptr->get_ec().message());
	}


	void WebSocketServerEndPoint::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		wspp::ConnectionPtr cptr = mEndPoint->get_con_from_hdl(connection);
		connectionFailed(WebSocketConnection(connection),
			cptr->get_ec().value(),
			cptr->get_ec().message());
	}


	void WebSocketServerEndPoint::onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg)
	{
		messageReceived(WebSocketConnection(con), WebSocketMessage(msg));
		send("who's your daddy now??", con, msg->get_opcode());
	}


	void WebSocketServerEndPoint::onHTTP(wspp::ConnectionHandle con)
	{
		wspp::ConnectionPtr conp = mEndPoint->get_con_from_hdl(con);
		std::string res = conp->get_request_body();
		std::stringstream ss;
		ss << "got HTTP request with " << res.size() << " bytes of body data.";
		conp->set_body(ss.str());
		conp->set_status(websocketpp::http::status_code::ok);
	}


	bool WebSocketServerEndPoint::onValidate(wspp::ConnectionHandle con)
	{
		// TODO: Validate incoming connection here, ie: accept or reject.
		return true;
	}
}
