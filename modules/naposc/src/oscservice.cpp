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
#include "oscinputcomponent.h"

RTTI_DEFINE(nap::OSCService)

namespace nap
{
	OSCService::~OSCService()
	{ }


	bool OSCService::init(nap::utility::ErrorState& errorState)
	{
		ResourceManagerService* resource_manager = getCore().getService<ResourceManagerService>();
		assert(resource_manager != nullptr);
		return true;
	}


	void OSCService::update()
	{
		// Forward every event to every input component of interest
		for (auto& receiver : mReceivers)
		{
			receiver->consumeEvents();

			// Keep forwarding events until the queue runs out
			while (receiver->hasEvents())
			{
				const OSCEvent& osc_event = receiver->currentEvent();
				for (const auto& input_comp : mInputs)
				{
					// Empty (no specified address) always receives the message
					if (input_comp->mAddresses.empty())
					{
						input_comp->trigger(osc_event);
						continue;
					}

					// Try to match the address
					for (const auto& address : input_comp->mAddresses)
					{
						if (address == osc_event.mAddress)
						{
							input_comp->trigger(osc_event);
							break;
						}
					}
				}
				receiver->popEvent();
			}
		}
	}


	void OSCService::collectInputComponents(const EntityInstance& entity, std::vector<OSCInputComponentInstance*>& components)
	{
		// Add all possible input components
		entity.getComponentsOfType<OSCInputComponentInstance>(components);
		
		// Sample children
		for (const auto& child_entity : entity.getChildren())
		{
			collectInputComponents(*child_entity, components);
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


	void OSCService::registerInputComponent(OSCInputComponentInstance& input)
	{
		mInputs.emplace_back(&input);
	}


	void OSCService::removeInputComponent(OSCInputComponentInstance& input)
	{
		auto found_it = std::find_if(mInputs.begin(), mInputs.end(), [&](const auto& it)
		{
			return it == &input;
		});
		assert(found_it != mInputs.end());
		mInputs.erase(found_it);
	}

}