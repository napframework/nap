// Local includes
#include "oscpacketlistener.h"

// External includes
#include <iostream>

namespace nap
{
	void OSCPacketListener::ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint)
	{
		std::cout << "message received" << "\n";
	}
}

