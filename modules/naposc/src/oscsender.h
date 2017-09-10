#pragma once

// External Includes
#include <ip/UdpSocket.h>
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
	class OSCBuffer;
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
		 * Note that it's more efficient to add a message to the queue and send them as a bundle
		 * @param oscEVent the event to send
		 */
		bool send(const OSCEvent& oscEvent);

		/**
		 * Adds an event to the queue
		 * This is more efficient than sending over an individual message
		 * Call send() to send over the queue as one message bundle
		 * @param oscEvent the event to add to the queue, note that the sender
		 * takes ownership of the event
		 */
		void addEvent(OSCEventPtr oscEvent);

		/**
		 * Sends all the osc events in the queue as a bundle
		 * All events that were send are removed from the queue
		 */
		void send();

	private:
		// Socket used for sending messages
		std::unique_ptr<UdpTransmitSocket> mSocket = nullptr;

		/**
		 * Adds an oscEvent to the package this is send
		 * @param outPacket the packet to populate
		 */
		bool addEvent(const OSCEvent& oscEvent, osc::OutboundPacketStream& outPacket, utility::ErrorState& error);

		// Queue that holds all the events to be send over as a bundle
		std::queue<OSCEventPtr> mEventQueue;
		
		// The number of bytes represented by the values in the event queue
		std::size_t	mEventQueueDataSize;

		// Vector that holds osc data to be send over
		std::unique_ptr<OSCBuffer> mBuffer;
	};


	/**
	* Simple struct that is used by the sender to allocate OSC data
	* The buffer grows based on the required elements (but never shrinks)
	*/
	class OSCBuffer final
	{
	public:
		// Default constructor
		OSCBuffer(std::size_t elements)					{ reserve(elements); }

		// Clears the buffer if data was allocated
		~OSCBuffer();

		/**
		* Reserves x number of elements for the buffer
		* Note that if the number of elements is less or equal to the size of the buffer
		* the buffer remains the same
		*/
		void reserve(std::size_t elements);

		// Buffer data
		char* mData = nullptr;

		// Size of the buffer
		std::size_t mSize = 0;
	};
}
