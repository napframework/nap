// Local Includes
#include "websocketserverendpoint.h"
#include "websocketserver.h"
#include "websocketticket.h"

// External Includes
#include <nap/logger.h>
#include <mathutils.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/error/en.h>

RTTI_BEGIN_ENUM(nap::WebSocketServerEndPoint::EAccessMode)
	RTTI_ENUM_VALUE(nap::WebSocketServerEndPoint::EAccessMode::EveryOne,	"Everyone"),
	RTTI_ENUM_VALUE(nap::WebSocketServerEndPoint::EAccessMode::Ticket,		"Ticket"),
	RTTI_ENUM_VALUE(nap::WebSocketServerEndPoint::EAccessMode::Reserved,	"Reserved")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::WebSocketServerEndPoint)
	RTTI_PROPERTY("AllowPortReuse",			&nap::WebSocketServerEndPoint::mAllowPortReuse,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LogConnectionUpdates",	&nap::WebSocketServerEndPoint::mLogConnectionUpdates,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port",					&nap::WebSocketServerEndPoint::mPort,						nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("AccessMode",				&nap::WebSocketServerEndPoint::mMode,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ConnectionLimit",		&nap::WebSocketServerEndPoint::mConnectionLimit,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("LibraryLogLevel",		&nap::WebSocketServerEndPoint::mLibraryLogLevel,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AllowControlOrigin",		&nap::WebSocketServerEndPoint::mAccessAllowControlOrigin,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Clients",				&nap::WebSocketServerEndPoint::mClients,					nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	WebSocketServerEndPoint::WebSocketServerEndPoint()
	{

	}

	bool WebSocketServerEndPoint::start(nap::utility::ErrorState& error)
	{
		// Run until stopped
		assert(!mRunning);

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
		mServerTask  = std::async(std::launch::async, std::bind(&WebSocketServerEndPoint::run, this));
		mRunning = true;

		return true;
	}


	bool WebSocketServerEndPoint::isOpen() const
	{
		return mRunning;
	}


	bool WebSocketServerEndPoint::send(const WebSocketConnection& connection, const std::string& message, EWebSocketOPCode code, nap::utility::ErrorState& error)
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


	bool WebSocketServerEndPoint::send(const WebSocketConnection& connection, void const* payload, int length, EWebSocketOPCode code, nap::utility::ErrorState& error)
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


	int WebSocketServerEndPoint::getConnectionCount()
	{
		std::lock_guard<std::mutex> lock(mConnectionMutex);
		return mConnections.size();
	}


	bool WebSocketServerEndPoint::acceptsNewConnections()
	{
		if (mConnectionLimit < 0)
			return true;

		std::lock_guard<std::mutex> lock(mConnectionMutex);
		return mConnections.size() < mConnectionLimit;
	}

	void WebSocketServerEndPoint::run()
	{
		// Start running until stopped
		mEndPoint.run();
	}


	void WebSocketServerEndPoint::stop()
	{
		if (mRunning)
		{
			// Stop listening for new connections
			std::error_code stdec;
			mEndPoint.stop_listening(stdec);
			if (stdec)
			{
				assert(false);
				nap::Logger::error("%s: %s", mID.c_str(), stdec.message().c_str());
			}

			// Close all client connections
			utility::ErrorState napec;
			if (!disconnect(napec))
			{
				assert(false);
				nap::Logger::error("%s: %s", mID.c_str(), napec.toString().c_str());
			}

			// Explicitly stop
			mEndPoint.stop();

			// Wait for thread to finish
			assert(mServerTask.valid());
			mServerTask.wait();
			mRunning = false;
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

		// Initiate logging
		mEndPoint.clear_error_channels(websocketpp::log::elevel::all);
		mEndPoint.set_error_channels(mLogLevel);

		mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
		mEndPoint.set_access_channels(mAccessLogLevel);
		
		// If the endpoint can be re-used by other processes
		mEndPoint.set_reuse_addr(mAllowPortReuse);

		// Init asio
		std::error_code stdec;
		mEndPoint.init_asio(stdec);
		if (stdec)
		{
			errorState.fail(stdec.message());
			return false;
		}
		
		// Install connection open / closed handlers
		mEndPoint.set_http_handler(std::bind(&WebSocketServerEndPoint::onHTTP, this, std::placeholders::_1));
		mEndPoint.set_open_handler(std::bind(&WebSocketServerEndPoint::onConnectionOpened, this, std::placeholders::_1));
		mEndPoint.set_close_handler(std::bind(&WebSocketServerEndPoint::onConnectionClosed, this, std::placeholders::_1));
		mEndPoint.set_fail_handler(std::bind(&WebSocketServerEndPoint::onConnectionFailed, this, std::placeholders::_1));
		mEndPoint.set_validate_handler(std::bind(&WebSocketServerEndPoint::onValidate, this, std::placeholders::_1));
		mEndPoint.set_ping_handler(std::bind(&WebSocketServerEndPoint::onPing, this, std::placeholders::_1, std::placeholders::_2));

		// Install message handler
		mEndPoint.set_message_handler(std::bind(
			&WebSocketServerEndPoint::onMessageReceived, this,
			std::placeholders::_1, std::placeholders::_2
		));

		// Create unique hashes out of tickets
		for (const auto& ticket : mClients)
			mClientHashes.emplace(ticket->toHash());
		
		return true;
	}
	

	void WebSocketServerEndPoint::onConnectionOpened(wspp::ConnectionHandle connection)
	{
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			return;
		}

		// Add to list of actively managed connections
		{
			std::lock_guard<std::mutex> lock(mConnectionMutex);
			mConnections.emplace_back(cptr);
		}

		connectionOpened(WebSocketConnection(connection));
	}


	void WebSocketServerEndPoint::onConnectionClosed(wspp::ConnectionHandle connection)
	{
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			return;
		}

		// Signal that it closed
		connectionClosed(WebSocketConnection(connection),
			cptr->get_ec().value(), 
			cptr->get_ec().message());

		// Remove from internal list of connections
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
	}


	void WebSocketServerEndPoint::onConnectionFailed(wspp::ConnectionHandle connection)
	{
		std::error_code stdec;
		wspp::ConnectionPtr cptr = mEndPoint.get_con_from_hdl(connection, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			return;
		}

		connectionFailed(WebSocketConnection(connection),
			cptr->get_ec().value(),
			cptr->get_ec().message());
	}


	void WebSocketServerEndPoint::onMessageReceived(wspp::ConnectionHandle con, wspp::MessagePtr msg)
	{
		messageReceived(WebSocketConnection(con), WebSocketMessage(msg));
	}


	void WebSocketServerEndPoint::onHTTP(wspp::ConnectionHandle con)
	{
		// Get handle to connection
		std::error_code stdec;
		wspp::ConnectionPtr conp = mEndPoint.get_con_from_hdl(con, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			conp->set_status(websocketpp::http::status_code::internal_server_error);
			return;
		}

		// set whether the response can be shared with requesting code from the given origin.
		conp->append_header("Access-Control-Allow-Origin", mAccessAllowControlOrigin);

		// Get request body
		std::string body = conp->get_request_body();
		
		// Try to parse as JSON
		rapidjson::Document document;
		rapidjson::ParseResult parse_result = document.Parse(body.c_str());
		if (!parse_result || !document.IsObject())
		{
			conp->set_status(websocketpp::http::status_code::bad_request, "unable to parse as JSON");
			return;
		}

		// Extract user information, this field is required.
		if (!document.HasMember("user"))
		{
			conp->set_status(websocketpp::http::status_code::bad_request, "missing member: 'user");
			return;
		}

		// Create ticket
		nap::WebSocketTicket ticket;
		ticket.mID = math::generateUUID();
		ticket.mUsername = document["user"].GetString();

		// Ensure user name is not empty
		if (ticket.mUsername.empty())
		{
			conp->set_status(websocketpp::http::status_code::bad_request, "member 'user' is empty");
			return;
		}
		
		// The password is optional and left empty when not specified
		if (document.HasMember("pass"))
			ticket.mPassword = document["pass"].GetString();
		
		// Convert to binary blob
		utility::ErrorState error;
		std::string ticket_str;
		if (!ticket.toBinaryString(ticket_str, error))
		{
			nap::Logger::error(error.toString());
			conp->set_status(websocketpp::http::status_code::internal_server_error);
			return;
		}

		// Set ticket as body
		conp->set_body(ticket_str);
		conp->set_status(websocketpp::http::status_code::ok);
	}


	bool WebSocketServerEndPoint::onValidate(wspp::ConnectionHandle con)
	{
		// Get connection handle
		std::error_code stdec;
		wspp::ConnectionPtr conp = mEndPoint.get_con_from_hdl(con, stdec);
		if (stdec)
		{
			nap::Logger::error(stdec.message());
			conp->set_status(websocketpp::http::status_code::internal_server_error);
			return false;
		}

		// Make sure we accept new connections
		if (!acceptsNewConnections())
		{
			conp->set_status(websocketpp::http::status_code::too_many_requests,
				"client connection count exceeded");
			return false;
		}

		// Get sub-protocol field
		const std::vector<std::string>& sub_protocol = conp->get_requested_subprotocols();
		
		// When every client connection is allowed no sub-protocol field must be defined.
		if (mMode == EAccessMode::EveryOne)
		{
			if (sub_protocol.empty())
				return true;

			// Sub-protocol requested but can't authenticate.
			conp->set_status(websocketpp::http::status_code::not_found, "invalid sub-protocol");
			return false;
		}

		// We have tickets and therefore specific clients we accept.
		// Use the sub_protocol to extract client information
		if (sub_protocol.empty() || sub_protocol[0].empty())
		{
			conp->set_status(websocketpp::http::status_code::non_authoritative_information,
				"unable to extract ticket");
			return false;
		}

		// Extract ticket
		conp->select_subprotocol(sub_protocol[0]);
		
		// Convert from binary into string
		WebSocketTicket client_ticket;
		utility::ErrorState error;
		if (!client_ticket.fromBinaryString(sub_protocol[0], error))
		{
			conp->set_status(websocketpp::http::status_code::non_authoritative_information,
				"first sub-protocol argument is not a valid ticket object");
			return false;
		}

		// Valid ticket was extracted, allowed access
		if (mMode == EAccessMode::Ticket)
			return true;

		// Locate ticket
		if (mClientHashes.find(client_ticket.toHash()) == mClientHashes.end())
		{
			conp->set_status(websocketpp::http::status_code::non_authoritative_information,
				"not a valid ticket");
			return false;
		}

		// Welcome!
		return true;
	}


	bool WebSocketServerEndPoint::onPing(wspp::ConnectionHandle con, std::string msg)
	{
		return true;
	}


	bool WebSocketServerEndPoint::disconnect(nap::utility::ErrorState& error)
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
}
