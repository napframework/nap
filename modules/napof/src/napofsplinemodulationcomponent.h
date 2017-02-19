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
		Max = Triangle
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
		NumericAttribute<float>	mFrequency =		{ this, "Frequency", 1.0f, 1.0f, 100.0f };
		NumericAttribute<float>	mSpeed =			{ this, "Speed", 0.0f, 0.0f, 1.0f };
		NumericAttribute<float>	mOffset =			{ this, "Offset", 0.0f, 0.0f, 1.0f };
		NumericAttribute<float>	mAmplitude =		{ this, "Amplitude", 0.0f, 0.0f, 10.0f };
		Attribute<LfoType> mType =					{ this, "Type", LfoType::Sine };
		NumericAttribute<int> mIndex =				{ this, "LfoIndex", 0, 0, (int)LfoType::Max };

		// Slots
		void indexChanged(AttributeBase& attr)		{ mType.setValue(static_cast<LfoType>(attr.getValue<int>())); }
		NSLOT(mIndexChanged, AttributeBase&, indexChanged)

	private:
		float mTime = 0.0f;
		float mPreviousTime;
	};
}


RTTI_DECLARE(nap::OFSplineLFOModulationComponent)
