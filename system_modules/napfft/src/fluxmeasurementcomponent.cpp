/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fluxmeasurementcomponent.h"
#include "fftaudionodecomponent.h"
#include "fftutils.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::FluxMeasurementComponent)
	RTTI_PROPERTY("Scale",		&nap::FluxMeasurementComponent::mScale,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Decay",		&nap::FluxMeasurementComponent::mDecay,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinHertz",	&nap::FluxMeasurementComponent::mMinHz,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxHertz",	&nap::FluxMeasurementComponent::mMaxHz,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FluxMeasurementComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	// Always ensure a decreasing gradient
	static const float sMaximumDecay = 0.95f;


	//////////////////////////////////////////////////////////////////////////
	// FluxMeasurementComponent
	//////////////////////////////////////////////////////////////////////////

	void FluxMeasurementComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(FFTAudioNodeComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// FluxMeasurementComponentInstance
	//////////////////////////////////////////////////////////////////////////

	bool FluxMeasurementComponentInstance::init(utility::ErrorState& errorState)
	{
		// Fetch resource
		mResource = getComponent<FluxMeasurementComponent>();

		// Fetch audio service
		mAudioService = getEntityInstance()->getCore()->getService<audio::AudioService>();
		assert(mAudioService != nullptr);

		// Ensure FFTAudioComponentInstance is available
		mFFTAudioComponent = &getEntityInstance()->getComponent<FFTAudioNodeComponentInstance>();
		if (!errorState.check(mFFTAudioComponent != nullptr, "Missing nap::FFTAudioComponentInstance under entity"))
			return false;

		// Validate min/max hertz
		if (!errorState.check(mResource->mMinHz < mResource->mMaxHz, "%s: Invalid filter parameter item. Minimum hertz higher than maximum hertz.", mResource->mID.c_str()))
			return false;

		mMinHz = std::clamp(mResource->mMinHz, 0.0f, mAudioService->getNodeManager().getSampleRate());
		mMaxHz = std::clamp(mResource->mMaxHz, 0.0f, mAudioService->getNodeManager().getSampleRate());

		mScale = std::max(mResource->mScale, 0.0f);
		mDecay = std::clamp(mResource->mDecay, 0.0f, 1.0f);

		uint bin_count = mFFTAudioComponent->getFFTBuffer().getBinCount();
		mPreviousBuffer.resize(bin_count);

		return true;
	}


	void FluxMeasurementComponentInstance::update(double deltaTime)
	{
		// Fetch amplitudes
		const auto& amps = mFFTAudioComponent->getFFTBuffer().getAmplitudeSpectrum();

		float delta_time = static_cast<float>(deltaTime);
		float value_inverse = 1.0f - std::max(mFlux, 0.0f);\

		// The value to decrement each frame
		float decrement = std::abs(value_inverse - value_inverse * std::pow(mDecay, delta_time));

		// Compute the cutoff bins
		float interval = utility::interval(mFFTAudioComponent->getFFTBuffer().getBinCount() - 1, mAudioService->getNodeManager().getSampleRate());
		uint min_bin = static_cast<uint>(mMinHz / interval);
		uint max_bin = static_cast<uint>(mMaxHz / interval);

		float raw_flux = utility::flux(amps, mPreviousBuffer, min_bin, max_bin) * mScale;
		mFlux = std::clamp(std::max(raw_flux, mFlux - decrement), 0.0f, 1.0f);

		// Copy
		mPreviousBuffer = amps;
	}
}
