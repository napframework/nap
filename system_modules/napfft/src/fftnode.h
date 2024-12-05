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
	 * Node to compute the FFT of an audio signal.
	 * Useful for equalizers and audioreactive parameters.
	 */
	class NAPAPI FFTNode : public audio::Node
	{
	public:
		/**
		 * @param audioService the NAP audio service.
		 * @param nodeManager the node manager this node must be registered to.
		 * @param overlaps the number of overlaps 
		 */
		FFTNode(audio::NodeManager& nodeManager, FFTBuffer::EOverlap overlaps = FFTBuffer::EOverlap::One);

		// Destructor
		virtual ~FFTNode();

		/**
		 * Inherited from audio::Node
		 */
		void process() override;

		/**
		 * @return the FFT buffer
		 */
		const FFTBuffer& getFFTBuffer() const				{ return *mFFTBuffer; }

		/**
		 * @return the FFT buffer
		 */
		FFTBuffer& getFFTBuffer()							{ return *mFFTBuffer; }

		// The input for the audio signal that will be analyzed
		audio::InputPin mInput = { this };

		// The FFT buffer
		std::unique_ptr<FFTBuffer> mFFTBuffer;
	};
}
