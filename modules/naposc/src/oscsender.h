#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <rtti/factory.h>
#include <queue>

// Local Includes
#include "oscevent.h"

// Forward declares
class UdpTransmitSocket;
namespace nap
{
	class OSCService;
}
namespace osc
{
	class OutboundPacketStream;
}

namespace nap 
{
	/**
	 * Objects that sends osc messages 
	 * The sender manages it's own connection that can be constructed
	 * using a target ip address and port
	 */
	class NAPAPI OSCSender : public rtti::RTTIObject
	{
		friend class OSCService;
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		OSCSender() = default;

		// Constructor used by factory
		OSCSender(OSCService& service);

		// Kills connection
		virtual ~OSCSender();

		// Property: target machine ip address
		std::string mIPAddress;

		// Property: target machine port
		int mPort = 8000;

		/**
		 * Initializes the sender and registers it with the OSCService
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Sends an OSC message immediately without adding it to the queue
		 * Note that it's more efficient to add a message to the queue. This
		 * ensures that all the messages are send as a bundle when processed
		 * @param oscEVent the event to send
		 */
		bool send(const OSCEvent& oscEvent);

	private:
		// Socket used for sending messages
		std::unique_ptr<UdpTransmitSocket> mSocket = nullptr;

		/**
		 * Adds an oscEvent to the package this is send
		 * @param outPacket the packet to populate
		 */
		bool addEvent(const OSCEvent& oscEvent, osc::OutboundPacketStream& outPacket, utility::ErrorState& error);
	};
}
