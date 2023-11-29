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

		// Number of overlaps
		enum EOverlap : uint
		{
			One		= 1,
			Three	= 3,
			Five	= 5,
			Seven	= 7,
			Nine	= 9,
			Eleven	= 11
		};

		// Constructor
		FFTBuffer(uint dataSize, EOverlap overlap = EOverlap::One);
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
		struct KissContextDeleter { void operator()(KissContext* ctx) const; };
		std::unique_ptr<KissContext, KissContextDeleter> mContext;

		std::vector<std::complex<float>> mComplexOut;
		std::vector<std::complex<float>> mComplexOutAverage;

		AmplitudeSpectrum mAmplitude;					//< magnitude (rho)
		PhaseSpectrum mPhase;							//< phase angle (theta)

		std::vector<float> mForwardHammingWindow;		//< preprocess samples for fft window
		float mHammingWindowSum = 0.0f;
		float mNormalizationFactor;

		// The sample buffer is accessed on both the audio and main thread. Use mutex for read/write.
		std::vector<float> mSampleBuffer;
		float* mSampleBufferHalfPtr = nullptr;
		std::mutex mSampleBufferMutex;

		std::vector<float> mSampleBufferWindowed;

		uint mBinCount;
		float mBinFrequency;

		EOverlap mOverlap;
		uint mHopSize;

		bool mDirty = false;
	};
}
