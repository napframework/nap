/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// external includes
#include "componentptr.h"
#include "component.h"
#include "udpserver.h"
#include "apiservice.h"
#include <apicomponent.h>
#include <parametersimple.h>
#include <parametercolor.h>

namespace nap
{
    class UDPReceiveComponentInstance;

    /**
     * UDPReceiveComponent receives packets from the UDP server and sends the data to the API service which turns it
     * into API events which will be dispatched on the main thread from the API component.
     *
     * This demonstrates both the use of the napapi module and napudp
     */
    class NAPAPI UDPReceiveComponent : public Component
    {
    RTTI_ENABLE(Component)
    DECLARE_COMPONENT(UDPReceiveComponent, UDPReceiveComponentInstance)
    public:
        /**
         * UDPReceiveComponent depends on APIComponent
         * @param components APIComponent will be added
         */
        void getDependentComponents(std::vector<rtti::TypeInfo> &components) const override;

        ResourcePtr<UDPServer> mServer;	                    ///< Property: "UDPServer" ResourcePtr to the UDPServer
        ComponentPtr<APIComponent> mAPIComponent;           ///< Property: "APIComponent" ComponentPtr to APIComponent
        ResourcePtr<APISignature> mTextMessageSignature;    ///< Property: "TextMessageSignature" ResourcePtr to the text message API signature
        ResourcePtr<APISignature> mColorMessageSignature;   ///< Property: "ColorMessageSignature" ResourcePtr to the color message API signature
        ResourcePtr<ParameterString> mTextParameter;        ///< Property: "TextParameter" ResourcePtr to the the string parameter
        ResourcePtr<ParameterRGBColor8> mColorParameter;    ///< Property: "ColorParameter" ResourcePtr to the color parameter
    };


    /**
     * Instance of UDPReceiveComponent
     */
    class NAPAPI UDPReceiveComponentInstance : public ComponentInstance
    {
    RTTI_ENABLE(ComponentInstance)
    public:
        /**
         * Constructor
         * @param entity reference to entity holding the component
         * @param resource the component resource
         */
        UDPReceiveComponentInstance(EntityInstance& entity, Component& resource) :
                ComponentInstance(entity, resource)									{ }

        /**
         * Initializes the UDPReceiveComponent. Connects slots to UDPServer and APIComponent
         * @param errorState returns any errors that occurred during initialization
         * @return true on success
         */
        virtual bool init(utility::ErrorState& errorState) override;

        /**
         * Disconnects slots from UDPServer and APIComponent
         */
        virtual void onDestroy() override;

        /**
         * Fill data with latest received data. Thread-safe
         * @param data reference string to be filled with latest received data
         */
        void getLastReceivedData(std::string& data);
    private:
        // Component Instance Pointer to APIComponent
        ComponentInstancePtr<APIComponent> mAPIComponent = { this, &UDPReceiveComponent::mAPIComponent };

        // Slot contains function pointer to onPacketReceived, connected on Init & disconnected onDestroy
        Slot<const UDPPacket&> mPacketReceivedSlot = { this, &UDPReceiveComponentInstance::onPacketReceived };
        void onPacketReceived(const UDPPacket& packet);

        // Resolved pointer to UDPServer
        UDPServer* mServer = nullptr;

        // Resolved pointer to APIService
        APIService* mAPIService = nullptr;

        // Resolved pointers to parameters
        ParameterString* mTextParameter = nullptr;
        ParameterRGBColor8* mColorParameter = nullptr;

        Slot<const APIEvent&> mColorMessageSlot = { this, &UDPReceiveComponentInstance::onColorMessage };
        void onColorMessage(const APIEvent& colorEvent);

        Slot<const APIEvent&> mTextMessageSlot = { this, &UDPReceiveComponentInstance::onTextMessage };
        void onTextMessage(const APIEvent& textEvent);

        // Store last received data
        std::mutex mMutex;
        std::string mLastReceivedData;
    };
}
