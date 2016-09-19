#pragma once

// nap includes
#include <nap/component.h>
#include <napofupdatecomponent.h>

// nap of includes
#include <napofattributes.h>

namespace nap
{
	// Lag component blends from current to target value
	class OFLagComponentBase : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:
		// Default constructor
		OFLagComponentBase();

		//@name Update
		virtual void onUpdate() override;

		//@name Performs the blend in derived classes
		virtual void UpdateTargetValue(float inTimeDifference) = 0;

		//@name Attributes
		Attribute<float>	mSmoothTime = { this, "Damp Time", 0.1f };
		Attribute<float>	mMaxSpeed =   { this, "Max Speed", 100.0f };

	protected:
		float mCurrentVel = 0.0f;
		float mPreviousTime;
	};



	// Templated lag component works on a nap attribute
	// Slowly moves the current value towards target value
	template<typename T>
	class OFLagComponent : public OFLagComponentBase
	{
		RTTI_ENABLE_DERIVED_FROM(OFLagComponentBase)
	public:
		//@name Attribute
		void			SetAttribute(Attribute<T>& inAttribute) { mAttribute = &inAttribute; mTargetValue.setValue(inAttribute.getValue()); }

		//@name Lagged value
		const T&		GetCurrentValue() const { return mCurrentValue; }

		// @name Target
		virtual void	UpdateTargetValue(float inTimeDifference) override;
		Attribute<T>	mTargetValue = { this, "Target Value" };

	private:
		//@name members
		Attribute<T>*	mAttribute = nullptr;
		T				mCurrentValue;
	};

	// Typedefs
	typedef OFLagComponent<float>	OFFloatLagComponent;
	typedef OFLagComponent<ofVec3f>	OFVec3LagComponent;
}

RTTI_DECLARE_BASE(nap::OFLagComponentBase)
RTTI_DECLARE(nap::OFVec3LagComponent)
RTTI_DECLARE(nap::OFFloatLagComponent)
