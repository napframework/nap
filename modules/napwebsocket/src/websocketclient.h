#pragma once

// Local Includes
#include "websocketclientendpoint.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 * websocketclient
	 */
	class NAPAPI IWebSocketClient : public Resource
	{
		friend class WebSocketClientEndPoint;
		RTTI_ENABLE(Resource)
	public:
		virtual ~IWebSocketClient();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return if the client is connected
		 */
		bool isConnected() const;

		ResourcePtr<WebSocketClientEndPoint> mEndPoint;					///< Property: 'EndPoint' the client endpoint that manages all connections.
		std::string mURI;												///< Property: "UIR" Server URI to open connection to.

	protected:
		WebSocketConnection mConnection;								///< Websocket connection

	private:
		nap::Signal<const WebSocketConnection&> destroyed;
	};
}
