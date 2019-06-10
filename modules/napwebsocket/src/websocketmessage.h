#pragma once

// Local Includes
#include "wspp.h"
#include "websocketutils.h"

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	// Forward Declares
	class WebSocketServerEndPoint;

	/**
	 * A web-socket message that is received by an end-point or sent to an end-point.
	 */
	class NAPAPI WebSocketMessage final
	{
		friend class WebSocketServerEndPoint;
		friend class WebSocketClientEndPoint;
		friend class WebSocketClientWrapper;
	public:
		/**
		 * Construct a new web socket message using the given payload, opcode and finalize bit flag.
		 * @param message the payload associated with this message.
		 * @param code the op code that describes the intent of this message
		 * @param fin if this is the final message
		 */
		WebSocketMessage(const std::string& message, EWebSocketOPCode code, bool fin = true);

		/**
		 * Construct a new web socket message using the given payload, opcode and finalize bit flag.
		 * @param message the payload associated with this message.
		 * @param code the op code that describes the intent of this message
		 * @param fin if this is the final message
		 */
		WebSocketMessage(std::string&& message, EWebSocketOPCode code, bool fin = true);

		// no default constructor
		WebSocketMessage() = delete;
		// Move constructor
		WebSocketMessage(WebSocketMessage&& other);
		// Move assignment operator
		WebSocketMessage& operator=(WebSocketMessage&& other);
		// Default copy constructor
		WebSocketMessage(const WebSocketMessage& other) = default;
		// Default copy assignment operator
		WebSocketMessage& operator=(const WebSocketMessage& other) = default;

		/**
		 * @return a reference to the message's payload string
		 */
		const std::string& getPayload() const;

		/**
		 * Adds an extra set of characters to the already existing message.
		 * @param payload the string to append.
		 */
		void appendPayload(const std::string& payload);

		/**
		 * @return the message opcode that describes the purpose of this message.
		 */
		EWebSocketOPCode getCode() const;

		/**
		 * A message with the fin bit set will be sent as the last message of its
		 * sequence. A message with the fin bit cleared will require subsequent
		 * frames of opcode continuation until one of them has the fin bit set.
		 * @return if the fin (final) bit is set
		 */
		bool getFin() const;

	private:
		/**
		 * Constructs this message based on the a received websocketpp message
		 * @param message the websocketpp message
		 */
		WebSocketMessage(wspp::MessagePtr message);

		std::string mMessage;							///< The received or to be sent message.
		EWebSocketOPCode mCode;							///< OP code of the message.
		bool mFin = true;								///< If the message fin bit is or should be set.
	};
}
