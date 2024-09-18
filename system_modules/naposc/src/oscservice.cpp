/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <ip/PacketListener.h>
#include <osc/OscReceivedElements.h>
#include <osc/OscPrintReceivedElements.h>
#include <ip/UdpSocket.h>

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "oscservice.h"
#include "oscreceiver.h"
#include "oscinputcomponent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OSCService, "Processes OSC messages in NAP")
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	OSCService::OSCService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	OSCService::~OSCService()
	{ }


	bool OSCService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void OSCService::update(double deltaTime)
	{
		std::queue<OSCEventPtr> events;

		// Forward every event to every input component of interest
		for (auto& receiver : mReceivers)
		{
			receiver->consumeEvents(events);

			// Keep forwarding events until the queue runs out
			while (!(events.empty()))
			{
				OSCEvent& osc_event = *(events.front());
				for (const auto& input_comp : mInputs)
				{
					// Empty (no specified address) always receives the message
					if (input_comp->mAddressFilter.empty())
					{
						input_comp->trigger(osc_event);
						continue;
					}

					// Try to match the address
					for (const auto& address : input_comp->mAddressFilter)
					{
						if (nap::utility::startsWith(osc_event.getAddress(), address))
						{
							input_comp->trigger(osc_event);
							break;
						}
					}
				}
				events.pop();
			}
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
