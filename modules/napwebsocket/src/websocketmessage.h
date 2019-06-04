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
	class WebSocketServer;

	/**
	 * Utility class that wraps a websocketpp message.
	 * Can only be constructed by a web-socket endpoint.
	 * This message can be copied and moved freely.
	 */
	class NAPAPI WebSocketMessage final
	{
		friend class WebSocketServerEndPoint;
	public:
		// Default constructor
		WebSocketMessage() = delete;
		// Default move constructor
		WebSocketMessage(WebSocketMessage&& other) = default;
		// Default move assignment operator
		WebSocketMessage& operator=(WebSocketMessage&& other) = default;
		// Default copy constructor
		WebSocketMessage(WebSocketMessage& other) = default;
		// Default copy assignment operator
		WebSocketMessage& operator=(const WebSocketMessage& other) = default;

		/**
		 * @return a const reference to the message's payload string
		 */
		const std::string& getPayload() const;

		/**
		 * @return the message opcode
		 */
		EWebSocketOPCode getOPCode() const;

		/**
		 * A message with the fin bit set will be sent as the last message of its
		 * sequence. A message with the fin bit cleared will require subsequent
		 * frames of opcode continuation until one of them has the fin bit set.
		 * @return if the fin (final) bit is set
		 */
		bool getFin() const;

		/**
		 * @return if this message should be or is compressed
		 */
		bool isCompressed() const;

	private:
		WebSocketMessage(wspp::MessagePtr message);
		wspp::MessagePtr mMessage = nullptr;							///< Shared pointer to the message
	};
}
