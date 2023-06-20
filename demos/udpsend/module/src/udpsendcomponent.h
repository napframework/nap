/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// external includes
#include "componentptr.h"
#include "component.h"
#include "udpclient.h"
#include "apiservice.h"
#include <apicomponent.h>
#include <parameternumeric.h>
#include <parameter.h>
#include <parametersimple.h>
#include <parametercolor.h>
#include <apimessage.h>

namespace nap
{
    class UDPSendComponentInstance;

    /**
     * UDPSendComponent listens to changes of the assign ParameterString and ParameterColor and converts them into
     * API messages according to the assigned signatures.
     * API Events will be serialized in JSON format and send as UDP packets using the UDP Client
     *
     * This demonstrates both the use of the napapi module and napudp
     */
    class NAPAPI UDPSendComponent : public Component
    {
    RTTI_ENABLE(Component)
    DECLARE_COMPONENT(UDPSendComponent, UDPSendComponentInstance)
    public:

        ResourcePtr<UDPClient>              mClient;
        ResourcePtr<APISignature>           mTextMessageSignature;
        ResourcePtr<APISignature>           mColorMessageSignature;
        ResourcePtr<ParameterString>        mMessageParam;
        ResourcePtr<ParameterRGBColor8>     mColorParam;
    };

    /**
     * Instance of UDPSendComponent
     */
    class NAPAPI UDPSendComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        /**
         * Constructor
         * @param entity reference to entity holding the component
         * @param resource the component resource
         */
        UDPSendComponentInstance(EntityInstance& entity, Component& resource) :
                ComponentInstance(entity, resource)									{ }

        /**
         * Initializes the UDPSendComponentInstance. Connects slots to parameters
         * @param errorState returns any errors that occurred during initialization
         * @return true on success
         */
        virtual bool init(utility::ErrorState& errorState) override;

        /**
         * Disconnects slots from UDPClient
         */
        virtual void onDestroy() override;
    private:
        /**
         * Serializes API message and sends data to UDPClient
         * @param apiMessage the  APIMessage to send
         */
        void send(APIMessage& apiMessage);

        // Slot and callback for color parameter change
        Slot<RGBColor8> mColorChangedSlot = { this, &UDPSendComponentInstance::onColorChanged };
        void onColorChanged(RGBColor8 value);

        // Slot and callback for text parameter change
        Slot<std::string> mMessageChangedSlot = { this, &UDPSendComponentInstance::onMessageChanged };
        void onMessageChanged(std::string value);

        // Resolved resources
        UDPClient*              mClient;
        ParameterString*        mMessageParam;
        ParameterRGBColor8*     mColorParam;
        APISignature*           mTextMessageSignature;
        APISignature*           mColorMessageSignature;
    };
}
