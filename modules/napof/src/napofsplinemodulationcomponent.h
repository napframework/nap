#pragma once

// NAP OF Includes
#include <napofattributes.h>
#include <napofupdatecomponent.h>

namespace nap
{
	enum class LfoType
	{
		Sine = 0,
		Square,
		Saw,
		Triangle,
		Max
	};

	/**
	@brief NAP OF spline modulation component

	Modulates a spline based on the specified type (noise / lfo)
	**/
	class OFSplineLFOModulationComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		// Default constructor
		OFSplineLFOModulationComponent();

		//@name Update function
		virtual void onUpdate() override;

		// Attributes
		Attribute<float>	mFrequency = { this, "Frequency", 1.0f };
		Attribute<float>	mSpeed = { this, "Speed", 0.0f };
		Attribute<float>	mOffset = { this, "Offset", 0.0f };
		Attribute<float>	mAmplitude = { this, "Amplitude", 0.0f };
		Attribute<LfoType>	mType = { this, "Type", LfoType::Sine };

	private:
		float mTime = 0.0f;
		float mPreviousTime;
	};
}


RTTI_DECLARE(nap::OFSplineLFOModulationComponent)
