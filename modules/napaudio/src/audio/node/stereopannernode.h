/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Node to perform equal power panning on a stereo signal
		 */
		class NAPAPI StereoPannerNode : public Node
		{
		public:
			StereoPannerNode(NodeManager& manager);
			
			/**
			 * @param value: 0 is far left, 1 is far right
			 */
			void setPanning(ControllerValue value);
			
			/**
			 * Left channel of the stereo input signal
			 */
			InputPin leftInput = {this};
			
			/**
			 * Right channel of the stereo input signal
			 */
			InputPin rightInput = {this};
			
			/**
			 * Left channel of the stereo output signal
			 */
			OutputPin leftOutput = {this};
			
			/**
			 * Right channel of the stereo output signal
			 */
			OutputPin rightOutput = {this};
		
		private:
			void process() override;
			
			std::atomic<ControllerValue> mNewPanning = {0.5};
			
			ControllerValue mPanning = 0.5f;
			ControllerValue mLeftGain = 0; // Gain factor of the left channel
			ControllerValue mRightGain = 0; // Gain factor of the right channel
		};
		
	}
}
