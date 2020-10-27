/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <component.h>
#include <nap/signalslot.h>
#include <oscevent.h>
#include <queue>

namespace nap
{
    // Forward Declare
    class OscHandlerComponentInstance;
   
	/**
	 * Component that converts incoming midi messages into a string and stores them for display later on.
	 */
    class NAPAPI OscHandlerComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(OscHandlerComponent, OscHandlerComponentInstance)
        
    public:
        OscHandlerComponent() : Component() { }
        
        /**
         * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
         * @param components the components this object depends on
         */
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        
    private:
    };

    
	/**
	* Instance part of the osc handler component. This object registers itself to an osc input component
	* And transforms incoming messages. In this case this component only converts the message to a string
	* that can be used for display later on
	*/
    class NAPAPI OscHandlerComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        OscHandlerComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        
        /**
         * @return the most recent osc messages
         */
        const std::vector<std::string>& getMessages();
        
    private:

		/**
		 * Slot that is connected to the osc input component that receives new messages
		 */
        Slot<const OSCEvent&> eventReceivedSlot = { this, &OscHandlerComponentInstance::onEventReceived };

		/**
		 * Called when the slot above is send a new message
		 * @param OSCEvent the new osc event
		 */
        void onEventReceived(const OSCEvent&);
        
        std::vector<std::string> mReceivedEvents;		///< Holds all the received events
    };
        
}
