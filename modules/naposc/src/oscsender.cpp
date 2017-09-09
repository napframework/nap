// Local includes
#include "oscsender.h"

// External includes
#include <ip/UdpSocket.h>
#include <nap/logger.h>
#include <osc/OscOutboundPacketStream.h>

RTTI_BEGIN_CLASS(nap::OSCSender)
RTTI_PROPERTY("IpAddress", &nap::OSCSender::mIPAddress, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Port", &nap::OSCSender::mPort, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// Max size in bytes of an OSC message
static const size_t initialPacketCapacity(1536);

namespace nap
{
	OSCSender::~OSCSender()
	{}


	bool OSCSender::init(utility::ErrorState& errorState)
	{
		// Make the buffer
		mBuffer = std::make_unique<OSCBuffer>(initialPacketCapacity);

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


	bool OSCSender::send(const OSCEvent& oscEvent)
	{
		std::size_t buffer_size = oscEvent.getSize();
		buffer_size += sizeof(osc::BeginMessage);
		buffer_size += sizeof(osc::EndMessage);

		// Grow the buffer based on the number of bytes that need allocation
		mBuffer->reserve(buffer_size);

		// Create packet
		osc::OutboundPacketStream packet(mBuffer->mData, mBuffer->mSize);

		// Clear and add event
		packet.Clear();
		utility::ErrorState error;
		if (!addEvent(oscEvent, packet, error))
		{
			nap::Logger::warn(error.toString());
		}
		else
		{
			mSocket->Send(packet.Data(), packet.Size());
		}

		// Send over
		return true;
	}


	void OSCSender::send()
	{
		if (mEventQueue.empty())
			return;

		// Create the buffer
		std::size_t buffer_size = mEventQueueDataSize;
		buffer_size += sizeof(osc::BeginMessage)	* mEventQueue.size();
		buffer_size += sizeof(osc::EndMessage)		* mEventQueue.size();
		buffer_size += sizeof(osc::BundleInitiator);
		buffer_size += sizeof(osc::BundleTerminator);

		// Create packet, grow buffer if necessary
		mBuffer->reserve(buffer_size);
		osc::OutboundPacketStream packet(mBuffer->mData, initialPacketCapacity);
		packet.Clear();

		// Signal we're going to send a bundle
		packet << osc::BeginBundle();

		// Add all events
		utility::ErrorState error;
		while (!(mEventQueue.empty()))
		{
			if (!addEvent(*(mEventQueue.front().get()), packet, error))
			{
				nap::Logger::warn(error.toString().c_str());
			}
			mEventQueue.pop();
		}

		// Signal end of bundle and send
		packet << osc::EndBundle;
		mSocket->Send(packet.Data(), packet.Size());

		// Delete the buffer and clear data
		mEventQueueDataSize = 0;
	}


	bool OSCSender::addEvent(const OSCEvent& oscEvent, osc::OutboundPacketStream& outPacket, utility::ErrorState& error)
	{
		// Make sure the message is valid
		if (!(error.check(oscEvent.mAddress.size() != 0, "No OSC address specified")))
		{
			assert(false);
			return false;
		}

		if (!(error.check(oscEvent.mAddress[0] == '/', "Invalid OSC address")))
		{
			assert(false);
			return false;
		}

		if (!(error.check(oscEvent.getCount() > 0, "No OSC arguments specified")))
		{
			assert(false);
			return false;
		}
		
		// Add start of message
		outPacket << osc::BeginMessage(oscEvent.mAddress.c_str());

		// Add every argument
		for (const auto& arg : oscEvent.getArguments())
		{
			arg->add(outPacket);
		}

		// Mark end of message
		outPacket << osc::EndMessage;
		return true;
	}


	void OSCSender::addEvent(OSCEventPtr oscEvent)
	{
		mEventQueueDataSize += (oscEvent->getSize());
		mEventQueue.emplace(std::move(oscEvent));
	}


	OSCBuffer::~OSCBuffer()
	{
		if (mData != nullptr)
			delete[] mData;
		mSize = 0;
	}


	void OSCBuffer::reserve(std::size_t elements)
	{
		assert(elements > 0);
		if (elements <= mSize)
			return;
		
		if (mData != nullptr)
			delete[] mData;

		mData = new char[elements];
		mSize = elements;
	}

}