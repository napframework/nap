#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * Websocket ticket created and issued by a nap::WebSocketServerEndPoint.
	 * This ticket can be used by a client to gain access to a web-socket server.
	 */
	class NAPAPI WebSocketTicket : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Converts this ticket into a binary string
		 */
		bool toBinaryString(std::string& outString, utility::ErrorState& error);

		std::string mUsername = "";			///< Property: 'UserName'
		std::string mPassword = "";			///< Property: 'Password'
	};
}
