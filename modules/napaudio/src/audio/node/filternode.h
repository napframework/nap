#pragma once

// Std includes
#include <atomic>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/utility/delay.h>
#include <audio/utility/dirtyflag.h>

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
			
			std::atomic<EMode> mMode = {EMode::LowPass};
			std::atomic<ControllerValue> mFrequency = {440.f};
			std::atomic<ControllerValue> mResonance = {0.f};
			std::atomic<ControllerValue> mBand = {100.f};
			std::atomic<ControllerValue> mGain = {1.f};
			DirtyFlag mIsDirty;
			
			// Filter coefficients
			ControllerValue a0, a1, a2, b1, b2;
			
			Delay mOutput; // Delay line storing the input signal.
			Delay mInput; // Delay line storing the output signal.
		};
		
	}
	
}
