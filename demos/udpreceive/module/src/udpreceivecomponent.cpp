/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpreceivecomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::UDPReceiveComponent run time class definition
RTTI_BEGIN_CLASS(nap::UDPReceiveComponent)
        RTTI_PROPERTY("Server",	        &nap::UDPReceiveComponent::mServer,	        nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("APIComponent",	&nap::UDPReceiveComponent::mAPIComponent,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::UDPReceiveComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPReceiveComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // nap::UDPReceiveComponent
    //////////////////////////////////////////////////////////////////////////

    void UDPReceiveComponent::getDependentComponents(std::vector<rtti::TypeInfo> &components) const
    {
        components.emplace_back(RTTI_OF(APIComponent));
    }

    //////////////////////////////////////////////////////////////////////////
    // nap::UDPReceiveComponentInstance
    //////////////////////////////////////////////////////////////////////////

    bool UDPReceiveComponentInstance::init(utility::ErrorState& errorState)
    {
        // Get component
        auto* resource = getComponent<UDPReceiveComponent>();

        // Resolve server
        mServer = resource->mServer.get();

        // Get API Service
        mAPIService = getEntityInstance()->getCore()->getService<APIService>();

        // Register slots to server and API Component
        mServer->registerListenerSlot(mPacketReceivedSlot);
        mAPIComponent->messageReceived.connect(mMessageReceivedSlot);

        return true;
    }


    void UDPReceiveComponentInstance::onDestroy()
    {
        // Remove slots
        mServer->removeListenerSlot(mPacketReceivedSlot);
        mAPIComponent->messageReceived.disconnect(mMessageReceivedSlot);
    }


    void UDPReceiveComponentInstance::onPacketReceived(const nap::UDPPacket &packet)
    {
        // Packet will be received on whichever thread is assigned to the UDP server
        std::string data = packet.toString();

        // Try to send message to API server, which will forward it as API events on the main thread, log any errors
        utility::ErrorState error_state;
        if(!mAPIService->sendMessage(data.c_str(), &error_state))
        {
            nap::Logger::error(error_state.toString());
        }

        // Copy received packet
        std::lock_guard<std::mutex> lock(mMutex);
        mLastReceivedData = data;
    }


    void UDPReceiveComponentInstance::onMessageReceived(const nap::APIEvent &event)
    {
        // Message received is called from API component on the main thread
        assert(event.getCount() > 0);
        const auto& argument = event.getArgument(0);

        assert(argument->isString());
        mLastReceivedMessage = argument->asString();
    }
}