/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "udpsendcomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>

// nap::UDPReceiveComponent run time class definition
RTTI_BEGIN_CLASS(nap::UDPSendComponent, "Sends text and color changes as api commands (messages) over a UDP socket")
        RTTI_PROPERTY("Client",					&nap::UDPSendComponent::mClient,				nap::rtti::EPropertyMetaData::Required, "Link to the UDP client that handles packet transport")
        RTTI_PROPERTY("MessageParam",			&nap::UDPSendComponent::mMessageParam,			nap::rtti::EPropertyMetaData::Required, "Link to the parameter that holds the text to send")
        RTTI_PROPERTY("ColorParam",				&nap::UDPSendComponent::mColorParam,			nap::rtti::EPropertyMetaData::Required, "Link to the parameter that holds the color to send")
        RTTI_PROPERTY("TextMessageSignature",   &nap::UDPSendComponent::mTextMessageSignature,	nap::rtti::EPropertyMetaData::Required, "Link to the signature that describes the layout of the text message")
        RTTI_PROPERTY("ColorMessageSignature",  &nap::UDPSendComponent::mColorMessageSignature,	nap::rtti::EPropertyMetaData::Required, "Link to the signature that describes the layout of the color message")
RTTI_END_CLASS

// nap::UDPReceiveComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UDPSendComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // nap::UDPReceiveComponentInstance
    //////////////////////////////////////////////////////////////////////////

    bool UDPSendComponentInstance::init(utility::ErrorState& errorState)
    {
        // Get component
        auto* resource = getComponent<UDPSendComponent>();

        // Resolve resources
        mClient = resource->mClient.get();
        mColorParam = resource->mColorParam.get();
        mMessageParam = resource->mMessageParam.get();
        mTextMessageSignature = resource->mTextMessageSignature.get();
        mColorMessageSignature = resource->mColorMessageSignature.get();

        // Connect slots
        mColorParam->valueChanged.connect(mColorChangedSlot);
        mMessageParam->valueChanged.connect(mMessageChangedSlot);

        // Sanity check all API signatures
        if(!errorState.check(resource->mTextMessageSignature->mArguments.size() > 0, "Text Message Signature must have at least one argument!"))
            return false;

        if(!errorState.check(RTTI_OF(std::string)==resource->mTextMessageSignature->mArguments[0]->getRepresentedType(), "Text Message Signature argument must be of type std::string!"))
            return false;

        if(!errorState.check(resource->mColorMessageSignature->mArguments.size() > 0, "Color Message Signature must have at least one argument!"))
            return false;

        if(!errorState.check(RTTI_OF(std::vector<int>)==resource->mColorMessageSignature->mArguments[0]->getRepresentedType(), "Color Message Signature argument must be of type std::vector<int>!"))
            return false;

        return true;
    }


    void UDPSendComponentInstance::onDestroy()
    {
        // Remove slots
        mColorParam->valueChanged.disconnect(mColorChangedSlot);
        mMessageParam->valueChanged.disconnect(mMessageChangedSlot);
    }


    void UDPSendComponentInstance::onColorChanged(nap::RGBColor8 value)
    {
        // Create API message with signature id
        APIMessage api_message;
        api_message.mName = "APIColorMessage";
        api_message.mID = mColorMessageSignature->mID;

        // Fill the message with a single API string containing message content
        assert(!mColorMessageSignature->mArguments.empty());
        auto api_value = APIIntArray(mColorMessageSignature->mArguments[0]->mName,
			{value.getRed(), value.getGreen(), value.getBlue()});
        api_value.mID = math::generateUUID();
        api_message.mArguments.emplace_back(&api_value);

        // send the api message
        send(api_message);
    }


    void UDPSendComponentInstance::onMessageChanged(std::string value)
    {
        // Create API message with signature id
        APIMessage api_message;
        api_message.mName = "APITextMessage";
        api_message.mID = mTextMessageSignature->mID;

        // Fill the message with a single API string containing message content
        assert(!mTextMessageSignature->mArguments.empty());
        auto api_value = APIString(mTextMessageSignature->mArguments[0]->mName, value);
        api_value.mID = math::generateUUID();
        api_message.mArguments.emplace_back(&api_value);

        // send the api message
        send(api_message);
    }


    void UDPSendComponentInstance::send(APIMessage& apiMessage)
    {
        // Serialize message to JSON & send it
        std::string data;
        utility::ErrorState error_state;
        if(apiMessage.toJSON(data, error_state))
        {
            mClient->send(data);
        }
		else
        {
            nap::Logger::error(error_state.toString());
        }
    }
}
