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

        ResourcePtr<UDPServer> mServer;	///< Property: "UDPServer" ResourcePtr to the UDPServer
        ComponentPtr<APIComponent> mAPIComponent; ///< Property: "APIComponent" ComponentPtr to APIComponent
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

        // Returns last received message received from APIComponent
        const std::string& getLastReceivedMessage() const{ return mLastReceivedMessage; }

        // Returns last received data received from UDPServer
        const std::string& getLastReceivedData() const{ return mLastReceivedData; }
    private:
        // Component Instance Pointer to APIComponent
        ComponentInstancePtr<APIComponent> mAPIComponent = { this, &UDPReceiveComponent::mAPIComponent };

        // Slot contains function pointer to onPacketReceived, connected on Init & disconnected onDestroy
        Slot<const UDPPacket&> mPacketReceivedSlot = { this, &UDPReceiveComponentInstance::onPacketReceived };
        void onPacketReceived(const UDPPacket& packet);

        // Slot contains function pointer to onMessageReceived, connected on Init & disconnected onDestroy
        Slot<const APIEvent&> mMessageReceivedSlot = { this, &UDPReceiveComponentInstance::onMessageReceived };
        void onMessageReceived(const APIEvent& event);

        // Resolved pointer to UDPServer
        UDPServer* mServer = nullptr;

        // Resolved pointer to APIService
        APIService* mAPIService = nullptr;

        // Last received message (from APIComponent) and last received data (from UDPServer)
        std::string mLastReceivedMessage = "";
        std::string mLastReceivedData = "";

        // Mutex is locked when data is copied to mLastReceivedData
        std::mutex mMutex;
    };
}
