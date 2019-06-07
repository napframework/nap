// Local Includes
#include "websocketconnection.h"

namespace nap
{

	WebSocketConnection::WebSocketConnection(wspp::ConnectionHandle connection) : mConnection(connection)
	{
	}


	bool WebSocketConnection::expired() const
	{
		return mConnection.expired();
	}

}