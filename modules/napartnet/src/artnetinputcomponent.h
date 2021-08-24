/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "artdmxpacketevent.h"

// External Includes
#include <component.h>
#include <nap/signalslot.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	
	class ArtNetInputComponentInstance;
	class ArtNetService;

	/**
	 * Resource part of the ArtNetInputComponent, receives ArtDmx packet events based on the net, subnet and universe.
	 * The Art-Net service forwards an event when the net, subnet and universe match with those in the ArtDmx packet.
	 * When Receive All is true, all events are forwarded.
	 */
	class NAPAPI ArtNetInputComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ArtNetInputComponent, ArtNetInputComponentInstance)
	public:
		uint8_t	mNet = 0;				///< Property: 'Net' the Net from which events should be received.
		uint8_t	mSubNet = 0;			///< Property: 'SubNet' the SubNet from which events should be received.
		uint8_t	mUniverse = 0;			///< Property: 'Universe' the Universe from which events should be received.
		bool	mReceiveAll = false;	///< Property: 'Receive All' when true, all events are forwarded.
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Instance part of the ArtNetInputComponent, receives ArtDmx packet events based on the net, subnet and universe.
	 * This component will forward any received ArtDmx packet events to listening components.
	 * Listen to the packetReceived signal to receive ArtDmx packet events that pass the filters set for this component.
	 * The Art-Net service forwards an event when the net, subnet and universe match with those in the ArtDmx packet.
	 * When Receive All is true, all events are forwarded.
	 */
	class NAPAPI ArtNetInputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		ArtNetInputComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }
		
		// Unregister with service
		virtual ~ArtNetInputComponentInstance();

		/**
		 * Initializes the component and copies over the component settings.
		 * @param errorState contains the error when the component could not be initialized
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		uint8_t	mNet;			///< Property: 'Net' the Net from which events should be received.
		uint8_t	mSubNet;		///< Property: 'SubNet' the SubNet from which events should be received.
		uint8_t	mUniverse;		///< Property: 'Universe' the Universe from which events should be received.
		bool	mReceiveAll;	///< Property: 'Receive All' when true, all events are forwarded.

		Signal<const ArtDmxPacketEvent&> packetReceived;	///< Triggered when the component receives an ArtDmx packet event
        
		/**
		 * Returns the signal that is called when an ArtDmx packet is received.
		 * @return the message received signal.
		 */
        Signal<const ArtDmxPacketEvent&>* getPacketReceived() { return &packetReceived; }

	protected:
		/**
		 * This is triggered by the service when a new ArtDmx packet is received
		 * @param event the ArtDmx packet event that will be forwarded by this component
		 */
		void trigger(const ArtDmxPacketEvent& event);

	private:
		friend class ArtNetService;

		ArtNetService* mService = nullptr;					// Art-Net Service set when initialized
	};
}
