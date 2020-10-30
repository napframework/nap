/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/audionodemanager.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Node to scale an audio signal
		 */
		class NAPAPI GainNode : public Node
		{
			RTTI_ENABLE(Node)
		
		public:
			GainNode(NodeManager& manager, ControllerValue initValue = 1, unsigned int stepCount = 64) : Node(manager), mGain(initValue, stepCount) { }
			
			/**
			 * The input to be scaled
			 */
			InputPin audioInput = {this};
			
			/**
			 * Outputs the scaled signal
			 */
			OutputPin audioOutput = {this};
			
			/**
			 * Sets the gain or scaling factor
			 * @param gain the new gain value
			 * @param smoothTime smoothtime in milliseconds
			 */
			void setGain(ControllerValue gain, TimeValue smoothTime);
			
			/**
			 * @return: the gain scaling factor
			 */
			ControllerValue getGain() const { return mGain.getValue(); }
		
		private:
			/**
			 * Calculate the output, perform the scaling
			 */
			void process() override;
			
			LinearSmoothedValue<ControllerValue> mGain; // Current multiplication factor of the output signal.
		};
		
	}
}
