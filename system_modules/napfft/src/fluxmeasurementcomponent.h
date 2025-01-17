/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "fftutils.h"

// External includes
#include <audio/service/audioservice.h>
#include <component.h>
#include <smoothdamp.h>

namespace nap
{
	class FluxMeasurementComponentInstance;
	class FFTAudioNodeComponentInstance;
			
	/**
	 * Component to measure flux of the audio signal from an audio component.
	 * Flux is measured by taking the difference between the RMS of the previous and current FFT.
	 * The frequency band to be measured can be specified using `MinHz` and `MaxHz`.
	 * The last frame's measurement minus a decrement value based on decay is compared against
	 * the most recent measurement to return a smooth signal with `getFlux()`.
	 */
	class NAPAPI FluxMeasurementComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(FluxMeasurementComponent, FluxMeasurementComponentInstance)	
	public:
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float mMinHz = 0.0f;			///< Property: 'MinHz' Minimum cutoff frequency
		float mMaxHz = 44100.0f;		///< Property: 'MaxHz' Maximum cutoff frequency

		float mScale = 1.0f;			///< Property: 'Scale' Scaling applied to raw result before clamping
		float mDecay = 0.95f;			///< Property: 'Decay' Decay factor applied to final result
	};
		
		
	/**
	 * Instance of component to measure onsets of the audio signal from an audio component.
	 */
	class NAPAPI FluxMeasurementComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Constructor
		FluxMeasurementComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

		// Initialize the component
		bool init(utility::ErrorState& errorState) override;

		/**
		 * Update this component
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the computed flux value between 0.0 and 1.0
		 */
		float getFlux() const								{ return mFlux; }

		/**
		 * Sets the decay factor
		 * @param the decay factor
		 */
		void setDecay(float decay)							{ mDecay = decay; }

		/**
		 * Sets the scaling factor
		 * @param the scaling factor
		 */
		void setScale(float scale)							{ mScale = scale; }

	private:
		FluxMeasurementComponent*			mResource = nullptr;
		audio::AudioService*				mAudioService = nullptr;
		FFTAudioNodeComponentInstance*		mFFTAudioComponent = nullptr;

		FFTBuffer::AmplitudeSpectrum		mPreviousBuffer;

		float								mMinHz = 0.0f;
		float								mMaxHz = 44100.0f;

		float								mScale = 1.0f;
		float								mDecay = 0.95f;
		float								mFlux = 0.0f;
	};
}
