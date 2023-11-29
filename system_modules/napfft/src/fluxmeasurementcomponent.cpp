/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fluxmeasurementcomponent.h"
#include "fftaudionodecomponent.h"
#include "fftutils.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::FluxMeasurementComponent::FilterParameterItem)
	RTTI_PROPERTY("Parameter", &nap::FluxMeasurementComponent::FilterParameterItem::mParameter, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Multiplier", &nap::FluxMeasurementComponent::FilterParameterItem::mMultiplier, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ThresholdDecay", &nap::FluxMeasurementComponent::FilterParameterItem::mThresholdDecay, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinHertz", &nap::FluxMeasurementComponent::FilterParameterItem::mMinHz, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxHertz", &nap::FluxMeasurementComponent::FilterParameterItem::mMaxHz, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SmoothTime", &nap::FluxMeasurementComponent::FilterParameterItem::mSmoothTime, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::FluxMeasurementComponent)
	RTTI_PROPERTY("Parameters", &nap::FluxMeasurementComponent::mParameters, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Enable", &nap::FluxMeasurementComponent::mEnable, nap::rtti::EPropertyMetaData::Default)
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

		// Ensure FFTAudioComponentInstance is available
		mFFTAudioComponent = &getEntityInstance()->getComponent<FFTAudioNodeComponentInstance>();
		if (!errorState.check(mFFTAudioComponent != nullptr, "Missing nap::FFTAudioComponentInstance under entity"))
			return false;

		const uint bin_count = mFFTAudioComponent->getFFTBuffer().getBinCount();
		mOnsetList.reserve(mResource->mParameters.size());
		for (auto& entry : mResource->mParameters)
		{
			if (!errorState.check(entry->mMinHz < entry->mMaxHz, "%s: Invalid filter parameter item. Minimum hertz higher than maximum hertz.", mResource->mID.c_str()))
				return false;

			mOnsetList.emplace_back(*entry);
		}

		mPreviousBuffer.resize(bin_count);
		return true;
	}


	void FluxMeasurementComponentInstance::update(double deltaTime)
	{
		if (!mResource->mEnable)
			return;

		const float delta_time = static_cast<float>(deltaTime);
		mElapsedTime += delta_time;

		// Fetch amplitudes
		const auto& amps = mFFTAudioComponent->getFFTBuffer().getAmplitudeSpectrum();

		for (auto& entry : mOnsetList)
		{
			const float interval = utility::interval(mFFTAudioComponent->getFFTBuffer().getBinCount(), 44100.0f);
			const uint min_bin = static_cast<uint>(entry.mMinHz / interval);
			const uint max_bin = static_cast<uint>(entry.mMaxHz / interval);

			float decay = (entry.mThresholdDecay != nullptr) ? entry.mThresholdDecay->mValue : 0.0001f;

			float value_inverse = 1.0f - std::clamp(entry.mOnsetValue, 0.0f, sMaximumDecay);
			float decrement = std::abs(value_inverse - value_inverse * std::pow(decay, delta_time));

			float mult = (entry.mMultiplier != nullptr) ? entry.mMultiplier->mValue : 1.0f;
			float flux = utility::flux(amps, mPreviousBuffer, min_bin, max_bin) * mult;

			entry.mOnsetValue = std::clamp(std::max(flux, entry.mOnsetValue - decrement), 0.0f, 1.0f);
			float value_smoothed = entry.mOnsetSmoother.update(entry.mOnsetValue, delta_time);
			entry.mParameter.setValue(value_smoothed * entry.mParameter.mMaximum);
		}

		// Copy
		mPreviousBuffer = amps;
	}
}
