/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <component.h>
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>
#include <audio/node/inputnode.h>
#include <audio/node/multiplynode.h>
#include <audio/node/controlnode.h>

namespace nap
{
	namespace audio
	{
		
		class AudioInputComponentInstance;
		
		
		/**
		 * Component to receive audio input from the audio interface.
		 * Can be used as input to an OutpuComponent of LevelMeterComponent.
		 */
		class NAPAPI AudioInputComponent : public AudioComponentBase
		{
		RTTI_ENABLE(AudioComponentBase)
		DECLARE_COMPONENT(InputComponent, AudioInputComponentInstance)
		public:
			AudioInputComponent() : AudioComponentBase()
			{}
			
			// Properties
			std::vector<int> mChannels; ///< property: 'Channels' Defines what audio input channels to receive data from. The size of this array determines the number of channels that this component will output.
			ControllerValue mGain = 1.0; ///< property: 'Gain' Overall gain.
		
		private:
		};
		
		
		/**
		 * Instance of component to receive audio input from the audio interface.
		 * Can be used as input to an OutputComponent of LevelMeterComponent.
		 */
		class NAPAPI AudioInputComponentInstance : public AudioComponentBaseInstance
		{
			RTTI_ENABLE(AudioComponentBaseInstance)
		public:
			AudioInputComponentInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(
					entity, resource)
			{}
			
			// Inherited from ComponentInstance
			bool init(utility::ErrorState& errorState) override;
			
			// Inherited from AudioComponentBaseInstance
			int getChannelCount() const override { return mGainNodes.size(); }
			
			OutputPin* getOutputForChannel(int channel) override { return &mGainNodes[channel]->audioOutput; }
			
			/**
			 * Set the input gain factor of the input signal.
			 */
			void setGain(ControllerValue gain);
			
			/**
			 * @return: Gain factor of the input signal.
			 */
			ControllerValue getGain() const;
		
		private:
			std::vector<SafeOwner<Node>> mInputNodes; // Nodes pulling audio input data out of the ADC inputs from the node manager
			std::vector<SafeOwner<MultiplyNode>> mGainNodes; // Nodes to control gain level of the input
			SafeOwner<ControlNode> mGainControl; // Node to control the gain for each channel.
			
			ControllerValue mGain = 1; // Gain factor of the output signal.
		};
		
	}
	
}
