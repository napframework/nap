/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/numeric.h>
#include <complex>
#include <mutex>

namespace nap
{
	/**
	 * Wraps a kiss context
	 */
	class NAPAPI FFTBuffer
	{
		RTTI_ENABLE()
	public:
		using AmplitudeSpectrum = std::vector<float>;
		using PhaseSpectrum = std::vector<float>;

		/**
		 * Number of overlaps (hops)
		 */
		enum class EOverlap : uint
		{
			One		= 1,
			Three	= 3,
			Five	= 5,
			Seven	= 7,
			Nine	= 9,
			Eleven	= 11
		};

		/**
		 * @param dataSize
		 * @param overlap
		 */
		FFTBuffer(uint dataSize, EOverlap overlap = EOverlap::One);

		// Destructor
		~FFTBuffer();

		/**
		 * Update the internal sample buffer to perform FFT on. This funtion is thread-safe.
		 */
		void supply(const std::vector<float>& samples);

		/**
		 * Performs FFT and updates amplitudes and phases. This funtion is thread-safe.
		 */
		void transform();

		/**
		 * @return the number of FFT bins
		 */
		uint getBinCount() const { return mBinCount; }

		/**
		 * @return the number of sample buffer data elements
		 */
		uint getDataSize();

		/**
		 * @return normalized magnitudes (rho)
		 */
		const AmplitudeSpectrum& getAmplitudeSpectrum();

		/**
		 * @return normalized phase angles (theta)
		 */
		const PhaseSpectrum& getPhaseSpectrum();

	private:
		class KissContext;
		std::unique_ptr<KissContext> mContext;

		std::vector<std::complex<float>> mComplexOut;				//< Complex
		std::vector<std::complex<float>> mComplexOutAverage;		//< Complex averaged over multiple analysis hops

		AmplitudeSpectrum mAmplitude;								//< Magnitude (rho)
		PhaseSpectrum mPhase;										//< Phase angle (theta)

		std::vector<float> mForwardHammingWindow;					//< Preprocessed window function coefficients
		float mHammingWindowSum;									//< The sum of all window function coefficients
		float mNormalizationFactor;									//< Inverse of the window sum (2/sum)


		std::mutex mSampleBufferMutex;								//< The mutex for accessing the sample buffer
		std::vector<float> mSampleBufferA;							//< Samples provided by the audio node
		std::vector<float> mSampleBufferB;							//< Thread safe copy of original samples
		std::vector<float> mSampleBufferFormatted;					//< The sample buffer before application of a window function
		std::vector<float> mSampleBufferWindowed;					//< The sample buffer after application of a window function

		uint mBinCount;												//< The number of FFT bins			
		float mBinFrequency;										//< The frequency each bin comprises

		EOverlap mOverlap;											//< The number of audio buffer overlaps for FFT analysis (hops)
		uint mHopSize;												//< The number of bins of a single hop

		std::atomic<bool> mDirty = { false };										//< Amplitudes dirty checking flag, prevents redundant FFT analyses 
	};
}
