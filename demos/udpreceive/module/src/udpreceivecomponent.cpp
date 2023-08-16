/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpreceivecomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <color.h>

// nap::UDPReceiveComponent run time class definition
RTTI_BEGIN_CLASS(nap::UDPReceiveComponent)
        RTTI_PROPERTY("Server",	                &nap::UDPReceiveComponent::mServer,					nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("APIComponent",	        &nap::UDPReceiveComponent::mAPIComponent,			nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("TextMessageSignature",   &nap::UDPReceiveComponent::mTextMessageSignature,	nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("ColorMessageSignature",  &nap::UDPReceiveComponent::mColorMessageSignature,	nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("TextParameter",			&nap::UDPReceiveComponent::mTextParameter,			nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("ColorParameter",			&nap::UDPReceiveComponent::mColorParameter,			nap::rtti::EPropertyMetaData::Required)
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

        // Sanity check the API signatures
        if(!errorState.check(resource->mTextMessageSignature->mArguments.size() > 0,
			"Text Message Signature must have at least one argument!"))
            return false;

        if(!errorState.check(RTTI_OF(std::string)==resource->mTextMessageSignature->mArguments[0]->getRepresentedType(),
			"Text Message Signature argument must be of type std::string!"))
            return false;

        if(!errorState.check(resource->mColorMessageSignature->mArguments.size() > 0,
			"Color Message Signature must have at least one argument!"))
            return false;

        if(!errorState.check(RTTI_OF(std::vector<int>)==resource->mColorMessageSignature->mArguments[0]->getRepresentedType(),
			"Color Message Signature argument must be of type std::vector<int>!"))
            return false;

        // Create callbacks for assigned signatures
        mAPIMessageHandlers.emplace(resource->mTextMessageSignature->mID, [this](const APIEvent& event){ onTextMessage(event); });
        mAPIMessageHandlers.emplace(resource->mColorMessageSignature->mID, [this](const APIEvent& event){ onColorMessage(event); });

        // resolve parameters
        mColorParameter = resource->mColorParameter.get();
        mTextParameter = resource->mTextParameter.get();

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

        // Store last received data
        std::lock_guard<std::mutex> lock(mMutex);
        mLastReceivedData = data;
    }


    void UDPReceiveComponentInstance::onMessageReceived(const nap::APIEvent &event)
    {
        auto it = mAPIMessageHandlers.find(event.getID());
        if(it!=mAPIMessageHandlers.end())
        {
            it->second(event);
        }else
        {
            nap::Logger::warn(utility::stringFormat("UDPReceiveComponent received API event with unknown ID : %s", event.getID().c_str()));
        }
    }


    void UDPReceiveComponentInstance::onTextMessage(const nap::APIEvent &textEvent)
    {
        assert(textEvent.getCount()==1);
        const auto& arg = textEvent.getArgument(0);

        assert(arg->isString());
        auto value = arg->asString();

        mTextParameter->setValue(value);
    }


    void UDPReceiveComponentInstance::onColorMessage(const nap::APIEvent &colorEvent)
    {
        assert(colorEvent.getCount()==1);
        const auto& arg = colorEvent.getArgument(0);

        assert(arg->isArray());
        auto values = arg->asArray<int>();

        assert(values->size()==3);
        RGBColor8 color((*values)[0], (*values)[1], (*values)[2]);
        mColorParameter->setValue(color);
    }
}
