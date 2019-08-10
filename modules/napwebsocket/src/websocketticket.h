#pragma once

// External Includes
#include <nap/resource.h>
#include <rtti/deserializeresult.h>

namespace nap
{
	struct WebSocketTicketHash;

	/**
	 * Created and issued by a nap::WebSocketServerEndPoint.
	 * A ticket is used by a server end-point to reject or accept a client connection request.
	 * For more information on authorization look refer to the nap::WebSocketServerEndPoint documentation.
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
		 * Converts this ticket into a binary string representation.
		 * @param outString contains the result of the conversion
		 * @param error contains the error if conversion fails.
		 * @return if conversion succeeded.
		 */
		bool toBinaryString(std::string& outString, utility::ErrorState& error);

		/**
		 * Creates a web-socket ticket based on a previously acquired binary string. 
		 * @param binaryString the binary string representation of a ticket.
		 * @param result contains the result of the de-serialization step. Ticket is destructed when results are out of scope.
		 * @param error contains the error if conversion fails.
		 * @return the web-socket ticket, nullptr if conversion fails
		 */
		static WebSocketTicket* fromBinaryString(const std::string& binaryString, rtti::DeserializeResult& result, utility::ErrorState& error);

		/**
		 * Creates a hash-able ticket based on the info in this ticket.
		 * @return hashable ticket representation.
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
