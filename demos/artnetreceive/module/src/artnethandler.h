/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // External includes
#include <component.h>
#include <componentptr.h>
#include <nap/signalslot.h>
#include <artnetevent.h>
#include <artnetinputcomponent.h>
#include <vector>

namespace nap
{
	// Forward Declare
	class ArtNetHandlerComponentInstance;

	/**
	 * Component that stores the most recent data of the Art-Net stream
	 */
	class NAPAPI ArtNetHandlerComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ArtNetHandlerComponent, ArtNetHandlerComponentInstance)

	public:
		ComponentPtr<ArtNetInputComponent> mInput;	///< Property: 'Input' the Art-Net input component from which to receive events
	};


	/**
	 * Instance part of the Art-Net handler component. This object registers itself to
	 * an Art-Net input component and stores the most recent data of the Art-Net stream.
	 */
	class NAPAPI ArtNetHandlerComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		ArtNetHandlerComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		 * Initializes the component and copies over the component settings.
		 * @param errorState contains the error when the component could not be initialized
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the most recent ArtNet data
		 */
		const std::vector<float>& getData();

		ComponentInstancePtr<ArtNetInputComponent> mInput = { this, &ArtNetHandlerComponent::mInput };

	private:

		/**
		 * Slot that is connected to the Art-Net input component that receives new packets
		 */
		Slot<const ArtNetEvent&> eventReceivedSlot = { this, &ArtNetHandlerComponentInstance::onEventReceived };

		/**
		 * Called when the slot above is sent a new packet
		 * @param ArtNetEvent the new Art-Net event
		 */
		void onEventReceived(const ArtNetEvent&);

		std::vector<float> mReceivedData;		///< Holds the most recent Art-Net data
	};
}
