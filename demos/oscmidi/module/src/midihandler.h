/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <component.h>
#include <nap/signalslot.h>
#include <midievent.h>
#include <queue>

namespace nap
{
    // Forward declare
    class MidiHandlerComponentInstance;
    
    /**
     * Component that converts incoming midi messages into a string and stores them for display later on.
     */
    class NAPAPI MidiHandlerComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(MidiHandlerComponent, MidiHandlerComponentInstance)
        
    public:
        MidiHandlerComponent() : Component() { }
        
        /**
         * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
         * @param components the components this object depends on
         */
        virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
        
    private:
    };

    
	/**
	 * Instance part of the midi handler component. This object registers itself to a midi input component
	 * And transforms incoming messages. In this case this component only converts the message to a string
	 * that can be used for display later on
	 */
    class NAPAPI MidiHandlerComponentInstance : public ComponentInstance
    {
        RTTI_ENABLE(ComponentInstance)
    public:
        MidiHandlerComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
        
        // Initialize the component
        bool init(utility::ErrorState& errorState) override;
        
        /**
         * Poll the component for received Midi messages
         */
        const std::vector<std::string>& getMessages();
        
    private:
		/**
		 * Slot connected to the midi received signal on initialization
		 */
        Slot<const MidiEvent&> eventReceivedSlot = { this, &MidiHandlerComponentInstance::onEventReceived };

		/**
		 *	Called by the slot when a new midi event is received
		 */
        void onEventReceived(const MidiEvent&);
        

        std::vector<std::string> mReceivedEvents;		///< Contains the latest received events
    };
        
}
