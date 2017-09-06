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
		ResourceManagerService* resource_manager = getCore().getService<ResourceManagerService>();
		assert(resource_manager != nullptr);
		mRootEntity = &(resource_manager->getRootEntity());
		return true;
	}


	void OSCService::processEvents(const EntityList& entities)
	{
		for (auto& receiver : mReceivers)
		{
			// Consume all received events
			receiver->consumeEvents();

			// Forward events to rest of system
			forwardEvents(*receiver, entities);
		}
	}


	void OSCService::processEvents()
	{
		assert(mRootEntity != nullptr);
		processEvents({ mRootEntity });
	}


	void OSCService::forwardEvents(OSCReceiver& receiver, const EntityList& entities)
	{
		while (receiver.hasEvents())
		{
			const OSCEvent& osc_event = receiver.getFrontEvent();
			receiver.popEvent();
		}
	}


	void OSCService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<OSCReceiverObjectCreator>(*this));
	}


	void OSCService::registerReceiver(OSCReceiver& receiver)
	{
		mReceivers.emplace_back(&receiver);
	}


	void OSCService::removeReceiver(OSCReceiver& receiver)
	{
		auto found_it = std::find_if(mReceivers.begin(), mReceivers.end(), [&](const auto& it)
		{
			return it == &receiver;
		});
		assert(found_it != mReceivers.end());
		mReceivers.erase(found_it);
	}
}