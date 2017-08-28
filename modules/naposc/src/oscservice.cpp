#include <ip/PacketListener.h>
#include <osc/OscReceivedElements.h>
#include <osc/OscPrintReceivedElements.h>
#include <ip/UdpSocket.h>

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>

// Local Includes
#include "oscservice.h"

RTTI_DEFINE(nap::OSCService)

static bool verbose = true;

class OscDumpPacketListener : public PacketListener {
public:
	virtual void ProcessPacket(const char *data, int size,
		const IpEndpointName& remoteEndpoint)
	{
		(void)remoteEndpoint; // suppress unused parameter warning

		std::cout << osc::ReceivedPacket(data, size);
	}
};


namespace nap
{
	OSCService::~OSCService()
	{

	}


	bool OSCService::init(nap::utility::ErrorState& errorState)
	{
		int port = 7000;

		OscDumpPacketListener listener;
		UdpListeningReceiveSocket s
		(
			IpEndpointName(IpEndpointName::ANY_ADDRESS, port),
			&listener
		);

		std::cout << "listening for input on port " << port << "...\n";
		return 0;
	}
}