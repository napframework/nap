/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/delay.h>
#include <audio/utility/dirtyflag.h>
#include <audio/utility/linearsmoothedvalue.h>

namespace nap
{
	namespace audio
	{
		
		/**
		 * Multi-purpose DSP filter using the Butterworth filter algorithm to calculate biquad coefficients.
		 * Is able to apply lowpass and highpass with or without variable resonance peak and bandpass filtering.
		 */
		class NAPAPI FilterNode : public Node
		{
		public:
			enum class EMode
			{
				LowPass, HighPass, BandPass, LowRes, HighRes
			};
		
		public:
			FilterNode(NodeManager& nodeManager) : Node(nodeManager), mOutput(8), mInput(8)
			{
				update();
			}
			
			// Inherited from Node
			void process() override;
			
			/**
			 * The input to be filtered
			 */
			InputPin audioInput = {this};
			
			/**
			 * Outputs the filtered signal
			 */
			OutputPin audioOutput = {this};

			/**
			 * Immediately changes the settings of the filter.
			 * Only call this before the filter starts being processed.
			 * @param frequency cutoff frequency of the filter in Hz
			 * @param resonanceBand In case of LowRes or HighRes: Resonance value. 0 means no resonance, 30 means self-oscillation. In case of bandpass: bandwith in herz.
			 */
			void prepare(ControllerValue frequency, ControllerValue resonanceBand, ControllerValue gain);
			
			/**
			 * Sets the mode of the filter.
			 * @param mode lowpass, highpass, bandpass, lowpass with resonance peak or highpass with resonance peak.
			 */
			void setMode(EMode mode);
			
			/**
			 * Sets the frequency parameter of the filter in Hz. For the different modes this means:
			 * - Cutoff frequency for highpass and lowpass filters
			 * - Center frequency for bandpass filtering
			 */
			void setFrequency(ControllerValue cutoffFrequency);
			
			/**
			 * Sets the resonance peak of the filter. 0 means no resonance, 30 means self-oscillation.
			 */
			void setResonance(ControllerValue resonance);
			
			/**
			 * Sets the bandwith for bandpass filtering in Hz.
			 */
			void setBand(ControllerValue band);
			
			/**
			 * Sets the gaining factor of the filter's output.
			 */
			void setGain(ControllerValue gain);
			
			/**
			 * @return the mode of the filter.
			 */
			EMode getMode() const { return mMode; }
			
			/**
			 * @return the cutoff frequency (for highpass and lowpass filters) or the center frequency (for bandpass filters) in Hz.
			 */
			ControllerValue getFrequency() const { return mFrequency; }
			
			/**
			 * @return the resonance peak for resonating low- and highpass filtering.
			 */
			ControllerValue getResonance() const { return mResonance; }
			
			/**
			 * @return the bandwidth in Hz for bandpass filtering.
			 */
			ControllerValue getBand() const { return mBand; }
			
			/**
			 * @return the output gain factor of the filter.
			 */
			ControllerValue getGain() const { return mGain; }
		
		private:
			void update();
			void calcCoeffs();
			
			std::atomic<EMode> mMode = {EMode::LowPass};
			std::atomic<ControllerValue> mFrequency = {440.f};
			std::atomic<ControllerValue> mResonance = {0.f};
			std::atomic<ControllerValue> mBand = {100.f};
			std::atomic<ControllerValue> mGain = {1.f};
			DirtyFlag mIsDirty;

			// Filter coefficients
			LinearSmoothedValue<ControllerValue> a0 = { 0, 64 };
			LinearSmoothedValue<ControllerValue> a1 = { 0, 64 };
			LinearSmoothedValue<ControllerValue> a2 = { 0, 64 };
			LinearSmoothedValue<ControllerValue> b1 = { 0, 64 };
			LinearSmoothedValue<ControllerValue> b2 = { 0, 64 };

			// Coeffecient destinations before update.
			ControllerValue a0Dest, a1Dest, a2Dest, b1Dest, b2Dest = 0;

			Delay mOutput; // Delay line storing the input signal.
			Delay mInput; // Delay line storing the output signal.
		};
		
	}
}
