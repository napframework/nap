#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	struct WebSocketTicketHash;

	/**
	 * Websocket ticket created and issued by a nap::WebSocketServerEndPoint.
	 * This ticket can be used by a client to gain access to a web-socket server.
	 */
	class NAPAPI WebSocketTicket : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		// Default constructor
		WebSocketTicket() = default;

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Converts this ticket into a binary string
		 */
		bool toBinaryString(std::string& outString, utility::ErrorState& error);

		/**
		 * Converts this from a binary string into a ticket
		 */
		bool fromBinaryString(const std::string& binaryString, utility::ErrorState& error);

		/**
		 * Creates a hashable ticket based on the info in this ticket
		 */
		WebSocketTicketHash toHash() const;

		std::string mUsername = "";			///< Property: 'UserName'
		std::string mPassword = "";			///< Property: 'Password'
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketTicketHash
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Hash-able web-socket ticket
	 */
	struct NAPAPI WebSocketTicketHash final
	{
	public:
		WebSocketTicketHash(const WebSocketTicket& ticket) :
			mHash(ticket.mUsername+ticket.mPassword)	{ }
		std::string mHash;

		bool operator== (const WebSocketTicketHash& rhs) const		{ return rhs.mHash == mHash; }

		bool operator!=(const WebSocketTicketHash& rhs) const		{ return !(rhs == *this); }
	};

}


// Hashes
namespace std
{
	template <>
	struct hash<nap::WebSocketTicketHash>
	{
		size_t operator()(const nap::WebSocketTicketHash& v) const
		{
			return hash<std::string>()(v.mHash);
		}
	};
}
