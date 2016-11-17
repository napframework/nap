#pragma once

// NAP Includes
#include <nap/component.h>
#include <nap/attribute.h>
#include <nap/signalslot.h>

#include <napofattributes.h>
#include <napofupdatecomponent.h>

// Of Includes Includes
#include <ofVec3f.h>
#include <ofMatrix4x4.h>
#include <utils/nofUtils.h>

namespace nap
{
	// Forward declares
	class OFService;

	/**
	@brief NAP OpenFrameworks transform
	**/
	class OFTransform : public ServiceableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(Component)
		friend class OFService;

	public:
		// Default constructor
		OFTransform();

		NumericAttribute<ofVec3f>		mTranslate{ this, "Translation", gOrigin,{ -100.0f, -100.0f, -100.0f },{ 100.0f, 100.0f, 100.0f }};
		NumericAttribute<ofVec3f>		mRotate{ this, "Rotation", gOrigin, {0.0f,0.0f,0.0f}, {360.0f, 360.0f, 360.0f} };
		NumericAttribute<ofVec3f>		mScale{ this, "Scale", {1.0f, 1.0f, 1.0f},	{ 0.0f, 0.0f, 0.0f },	{ 10.0f, 10.0f, 10.0f } };
		NumericAttribute<ofVec3f>		mPivot{ this, "Pivot", gOrigin, {0.0f, 0.0f, 0.0f}, {100.0f, 100.0f, 100.0f} };
		NumericAttribute<float>			mUniformScale{ this, "UniformScale", 1.0f, 0.0f, 1.0f };

		// Getters
		ofMatrix4x4					getLocalTransform() const;
		const ofMatrix4x4&			getGlobalTransform() const						{ return mGlobalTransform; }

	private:		
		// These are only called by the napofservice
		void						fetchChildTransforms();		//< Populates the list with child xforms
		void						update(const ofMatrix4x4& inParentMatrix);	//< Updates all child transforms

		// List of all child transform nodes
		std::vector<OFTransform*>	mChildTransforms;

		// Only accessable by the OFService
		ofMatrix4x4					mGlobalTransform;
	};


	/**
	@brief Rotates a transform based on the speed in axis x y z
	**/
	class OFRotateComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:

		// Default constructor
		OFRotateComponent();

		SignalAttribute	mReset			{ this, "Reset" };
		NumericAttribute<float>	mX		{ this, "SpeedX", 1.0f, 0.0f, 1.0f };
		NumericAttribute<float>	mY		{ this, "SpeedY", 1.0f, 0.0f, 1.0f };
		NumericAttribute<float>	mZ		{ this, "SpeedZ", 1.0f, 0.0f, 1.0f };
		NumericAttribute<float>	mSpeed	{ this, "Speed",  0.0f, 0.0f, 100.0f };

		// Overrides
		void onUpdate() override;

	private:
		// Timer
		float				mPreviousTime;
		float				mTimeX = 0.0f;
		float				mTimeY = 0.0f;
		float				mTimeZ = 0.0f;

		// OFTransform dependency
		ComponentDependency<OFTransform> mDependency = { this };

		// SLOTS
		void onReset(const SignalAttribute& signal);			//< Resets the rotatation
		void onUpdateChanged(const bool& value);				//< Stores the last time stamp
		NSLOT(mResetCalled, const SignalAttribute&, onReset)
		NSLOT(mUpdateCalled, const bool&, onUpdateChanged)
	};



	/**
	@brief Scales a transform based on the speed in axis x y z
	**/
	class OFScaleComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)
	public:
		OFScaleComponent();

		SignalAttribute			mReset		{ this, "Reset" };
		Attribute<bool>			mInstant	{ this, "Instant", false };
		NumericAttribute<float> mSpeed		{ this, "Speed", 0.5f, 0.0f, 1.0f };
		NumericAttribute<float>	mInfluence	{ this, "Influence", 0.0f, 0.0f, 1.0f };
		NumericAttribute<float> mScale		{ this, "Range", 1.0f, 0.0f, 25.0f };
		NumericAttribute<int>	mBlendMode	{ this, "BlendMode", 0, 0, 2 };

		// Overrides
		virtual void			onUpdate() override;

	private:
		// Timer
		float					mPreviousTime = 0.0f;
		float					mTime = 0.0f;

		bool					mRefSet = false;
		ofVec3f					mScaleRef = { 1.0f, 1.0f, 1.0f };

		// SLOTS
		void onReset(const SignalAttribute& signal);			//< Resets the rotatation
		void onUpdateChanged(const bool& value);				//< Stores the last time stamp
		NSLOT(mResetCalled, const SignalAttribute&, onReset)
		NSLOT(mUpdateCalled, const bool&, onUpdateChanged)

		// Component dependency on transform
		ComponentDependency<OFTransform> mDependency{ this };
	};
}

RTTI_DECLARE(nap::OFTransform)
RTTI_DECLARE(nap::OFRotateComponent)
RTTI_DECLARE(nap::OFScaleComponent)
