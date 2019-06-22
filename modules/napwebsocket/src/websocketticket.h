#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	struct WebSocketTicketHash;

	/**
	 * Websocket ticket created and issued by a nap::WebSocketServerEndPoint.
	 * A ticket is used by a server end-point to reject or accept a client connection request.
	 * For more information on authorization look at the nap::WebSocketServerEndPoint documentation.
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
		 * Creates a hash-able ticket based on the info in this ticket
		 */
		WebSocketTicketHash toHash() const;

		std::string mUsername = "";			///< Property: 'UserName'
		std::string mPassword = "";			///< Property: 'Password'
	};


	//////////////////////////////////////////////////////////////////////////
	// WebSocketTicketHash
	//////////////////////////////////////////////////////////////////////////

	/**
	 * WebSocketTicket hash representation
	 */
	struct NAPAPI WebSocketTicketHash final
	{
	public:
		WebSocketTicketHash(const WebSocketTicket& ticket);
	
		bool operator== (const WebSocketTicketHash& rhs) const		{ return rhs.mHash == mHash; }
		bool operator!=	(const WebSocketTicketHash& rhs) const		{ return !(rhs == *this); }

		std::string mHash;	///< Hash generated based on a ticket.
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
