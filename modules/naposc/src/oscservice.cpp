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
#include "oscreceiver.h"

RTTI_DEFINE(nap::OSCService)

namespace nap
{
	OSCService::~OSCService()
	{ }


	bool OSCService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void OSCService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<OSCReceiverObjectCreator>(*this));
	}
}