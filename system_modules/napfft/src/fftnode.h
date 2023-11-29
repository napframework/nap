/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "fftbuffer.h"

// Std includes
#include <atomic>

// NAP includes
#include <nap/resourceptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/safeptr.h>
#include <audio/core/process.h>

namespace nap
{
	// Forward declarations
	class AudioService;
		
	/**
	 * Node to measure the amplitude level of an audio signal.
	 * Can be used for VU meters or envelope followers for example.
	 * Can switch between measuring peaks of the signal or the root mean square.
	 */
	class NAPAPI FFTNode : public audio::Node
	{
	public:
		/**
		 * @param audioService: the NAP audio service.
		 * @param analysisWindowSize: the time window in milliseconds that will be used to generate one single output value. Also the period that corresponds to the analysis frequency.
		 * @param rootProcess: indicates that the node is registered as root process with the @AudioNodeManager and is processed automatically.
		 */
		FFTNode(audio::NodeManager& nodeManager, FFTBuffer::EOverlap overlaps = FFTBuffer::EOverlap::One);

		virtual ~FFTNode();

		/**
		 * Inherited from Node
		 */
		void process() override;

		/**
		 *
		 */
		const FFTBuffer& getFFTBuffer() const				{ return *mFFTBuffer; }

		/**
		 * 
		 */
		FFTBuffer& getFFTBuffer()							{ return *mFFTBuffer; }

		// The input for the audio signal that will be analyzed.
		audio::InputPin mInput = { this };

		// The FFT buffer
		std::unique_ptr<FFTBuffer> mFFTBuffer;
	};
}
