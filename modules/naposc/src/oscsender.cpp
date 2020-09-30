// Local includes
#include "oscsender.h"

// External includes
#include <ip/UdpSocket.h>
#include <nap/logger.h>
#include <osc/OscOutboundPacketStream.h>
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::OSCSender)
	RTTI_PROPERTY("IpAddress", &nap::OSCSender::mIPAddress, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Port", &nap::OSCSender::mPort, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BufferScale", &nap::OSCSender::mBufferScale, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// Max size in bytes of an OSC message
static const size_t initialPacketCapacity(1536);

namespace nap
{
	bool OSCSender::start(utility::ErrorState& errorState)
	{
		// Construct host endpoint
		IpEndpointName host(mIPAddress.c_str(), mPort);

		// Get address as string
		char hostIpAddress[IpEndpointName::ADDRESS_STRING_LENGTH];
		host.AddressAsString(hostIpAddress);

		// Create socket
		mSocket = std::make_unique<UdpTransmitSocket>(host);
		nap::Logger::info("Started OSC output connection, ip: %s, port: %d", hostIpAddress, mPort);

		return true;
	}


	void OSCSender::stop()
	{
		mSocket.reset(nullptr);
	}


	bool OSCSender::send(const OSCEvent& oscEvent)
	{
		std::size_t buffer_size = oscEvent.getSize();
		buffer_size *= math::max<int>(mBufferScale, 1);
		buffer_size += sizeof(osc::BeginMessage);
		buffer_size += sizeof(osc::EndMessage);

		// Grow the buffer based on the number of bytes that need allocation
		// Always allocate more than the bare minimum, that's why we multiply by 2.
		if (mBuffer.size() < buffer_size)
			mBuffer.resize(buffer_size);

		// Create packet
		osc::OutboundPacketStream packet(mBuffer.data(), mBuffer.size());

		// add event and send
		writeToPacket(oscEvent, packet);
		mSocket->Send(packet.Data(), packet.Size());

		// Send over
		return true;
	}


	void OSCSender::sendQueuedEvents()
	{
		if (mEventQueue.empty())
			return;

		// Create the buffer
		std::size_t buffer_size = mEventQueueDataSize;
		buffer_size *= math::max<int>(mBufferScale, 1);;
		buffer_size += sizeof(osc::BeginMessage)	* mEventQueue.size();
		buffer_size += sizeof(osc::EndMessage)		* mEventQueue.size();
		buffer_size += sizeof(osc::BundleInitiator);
		buffer_size += sizeof(osc::BundleTerminator);

		// Grow the buffer based on the number of bytes that need allocation
		if (mBuffer.size() < buffer_size)
			mBuffer.resize(buffer_size);

		// Create packet, grow buffer if necessary
		osc::OutboundPacketStream packet(mBuffer.data(), mBuffer.size());

		// Signal we're going to send a bundle
		packet << osc::BeginBundle();

		// Add all events
		while (!(mEventQueue.empty()))
		{
			writeToPacket(*(mEventQueue.front().get()), packet);
			mEventQueue.pop();
		}

		// Signal end of bundle and send
		packet << osc::EndBundle;
		mSocket->Send(packet.Data(), packet.Size());

		// Delete the buffer and clear data
		mEventQueueDataSize = 0;
	}


	void OSCSender::writeToPacket(const OSCEvent& oscEvent, osc::OutboundPacketStream& outPacket)
	{
		assert(oscEvent.getAddress().size() != 0);
		assert(oscEvent.getAddress()[0] == '/');
		
		// Add start of message
		outPacket << osc::BeginMessage(oscEvent.getAddress().c_str());

		// Add every argument
		for (const auto& arg : oscEvent.getArguments())
		{
			arg->add(outPacket);
		}

		// Mark end of message
		outPacket << osc::EndMessage;
	}


	void OSCSender::addEvent(OSCEventPtr oscEvent)
	{
		mEventQueueDataSize += (oscEvent->getSize());
		mEventQueue.emplace(std::move(oscEvent));
	}
}
