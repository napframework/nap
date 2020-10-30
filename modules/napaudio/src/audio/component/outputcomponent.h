/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>

namespace nap
{
	namespace audio
	{
		
		class OutputComponentInstance;
		
		
		/**
		 * Component that routs output from an audio component to the audio interface.
		 */
		class NAPAPI OutputComponent : public Component
		{
			RTTI_ENABLE(nap::Component)
			DECLARE_COMPONENT(OutputComponent, OutputComponentInstance)
		
		public:
			OutputComponent() : nap::Component()
			{}
		
		public:
			// Properties
			nap::ComponentPtr<AudioComponentBase> mInput; ///<  property: 'Input' The component whose audio output to rout to the interface
			
			std::vector<int> mChannelRouting; ///< property: 'Routing' The size of this vector indicates the number of channels this component outputs.
			///< Each element in the array represents one output channel on the audio interface.
			///< The value of the element indicates the channel from the input that will be routed to the corresponding output.
			///< A value of -1 means no output will be sent to the corresponding channel.
		};
		
		
		/**
		 * Instance of component that routs output from another audio component to the audio interface
		 */
		class NAPAPI OutputComponentInstance : public ComponentInstance
		{
		RTTI_ENABLE(nap::ComponentInstance)
		
		public:
			OutputComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity,
			                                                                                              resource)
			{}
			
			// Initialize the component
			bool init(utility::ErrorState& errorState) override;
			
			/**
			 * Connects a different audio component as input to be sent to the audio interface.
			 */
			void setInput(AudioComponentBaseInstance& input);
		
		private:
			std::vector<SafeOwner<Node>> mOutputs; // Nodes presenting the audio output to the node manager
			nap::ComponentInstancePtr<AudioComponentBase> mInput = {this,
			                                                        &OutputComponent::mInput}; // Pointer to the component whose input will be sent to the output.
			
			AudioService* mAudioService = nullptr; // Pointer to the audio service the audio output will be sent to.
			std::vector<int> mChannelRouting;
		};
		
	}
}
