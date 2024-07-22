/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpreceivecomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <color.h>

// nap::UDPReceiveComponent run time class definition
RTTI_BEGIN_CLASS(nap::UDPReceiveComponent, "Receives packets from the UDP server and forwards them to the api service as system commands (messages)")
        RTTI_PROPERTY("Server",	                &nap::UDPReceiveComponent::mServer,					nap::rtti::EPropertyMetaData::Required, "Link to the UDP server that handles packet transport")
        RTTI_PROPERTY("APIComponent",	        &nap::UDPReceiveComponent::mAPIComponent,			nap::rtti::EPropertyMetaData::Required, "Link to the API Component that deserializes and routes packet io")
        RTTI_PROPERTY("TextMessageSignature",   &nap::UDPReceiveComponent::mTextMessageSignature,	nap::rtti::EPropertyMetaData::Required, "Link to the signature that describes the layout of the text message")
        RTTI_PROPERTY("ColorMessageSignature",  &nap::UDPReceiveComponent::mColorMessageSignature,	nap::rtti::EPropertyMetaData::Required, "Link to the signature that describes the layout of the color message")
        RTTI_PROPERTY("TextParameter",			&nap::UDPReceiveComponent::mTextParameter,			nap::rtti::EPropertyMetaData::Required, "Link to the string parameter that receives the text message")
        RTTI_PROPERTY("ColorParameter",			&nap::UDPReceiveComponent::mColorParameter,			nap::rtti::EPropertyMetaData::Required, "Link to the color parameter that receives the color message")
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

        // Find the color message signature. A signature describes the interface of a message, ie:
        // The intent of the message and additional arguments. In this case we expect to find a
        // signature that has the ID of the color message signature.
        // When the callback is triggered you can safely extract the argument, the system
        // already validated and converted the message from you from json. The argument
        // contains the vector with int values that is used to update the RGB background color.
        const nap::APISignature* color_message_signature = mAPIComponent->findSignature(resource->mColorMessageSignature->mID);
        if (!errorState.check(color_message_signature != nullptr,
                              "%s: unable to find '%s' signature", this->mID.c_str(), resource->mColorMessageSignature->mID.c_str()))
        {
            errorState.fail("unable to install callback!");
            return false;
        }

        // Register callback, the slot calls onColorMessage.
        mAPIComponent->registerCallback(*color_message_signature, mColorMessageSlot);

        // Find the text message signature. The argument contains the string value that is used to update the text.
        const nap::APISignature* text_message_signature = mAPIComponent->findSignature(resource->mTextMessageSignature->mID);
        if (!errorState.check(text_message_signature != nullptr,
                              "%s: unable to find '%s' signature", this->mID.c_str(), resource->mTextMessageSignature->mID.c_str()))
        {
            errorState.fail("unable to install callback!");
            return false;
        }

        // Register callback, the slot calls onTextMessage.
        mAPIComponent->registerCallback(*text_message_signature, mTextMessageSlot);

        // resolve parameters
        mColorParameter = resource->mColorParameter.get();
        mTextParameter = resource->mTextParameter.get();

        // Register packet received slot to server
        mServer->registerListenerSlot(mPacketReceivedSlot);

        return true;
    }


    void UDPReceiveComponentInstance::onDestroy()
    {
        // Remove slots
        mServer->removeListenerSlot(mPacketReceivedSlot);
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


    void UDPReceiveComponentInstance::getLastReceivedData(std::string& data)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        data = mLastReceivedData;
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
