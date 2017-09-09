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
static const size_t oscPackageCapacity(1536);

namespace nap
{
	OSCSender::~OSCSender()
	{}


	bool OSCSender::init(utility::ErrorState& errorState)
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


	bool OSCSender::send(const OSCEvent& oscEvent)
	{
		// Create packet
		char buffer[oscPackageCapacity];
		osc::OutboundPacketStream packet(buffer, oscPackageCapacity);

		// Clear and add event
		packet.Clear();
		utility::ErrorState error;
		if (!addEvent(oscEvent, packet, error))
		{
			nap::Logger::warn(error.toString());
			return false;
		}

		// Send over
		mSocket->Send(packet.Data(), packet.Size());
		return true;
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

		if (!(error.check(oscEvent.getSize() > 0, "No OSC arguments specified")))
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
}