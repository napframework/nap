#pragma once

// Of Includes
#include <Spline/nofSpline.h>

// NAP Includes
#include <nap/component.h>
#include <nap/attribute.h>
#include <nap/signalslot.h>

#include <nap/configure.h>
#include <napofattributes.h>

// Local Includes
#include <napofrendercomponent.h>
#include <napofupdatecomponent.h>


namespace nap
{
	/**
	@brief NAP OF spline component
	**/
	class OFSplineComponent : public OFRenderableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFRenderableComponent)

	public:
		// Default constructor
		OFSplineComponent()						{ }

		// Spline attribute
		Attribute<NSpline>		mSpline			{ this, "Spline" };				//< The spline the component manages

		// Utility
		void					SetFromPolyLine(const ofPolyline& inPolyline);
		uint					GetPointCount() { return mSpline.getValueRef().GetPointCount(); }

		// Draw
		virtual void			onDraw() override;
	};



	/**
	@brief NAP OF spline component
	**/
	class OFSplineUpdateGPUComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		// Default constructor
		OFSplineUpdateGPUComponent()	{ }

		//@name Update function
		virtual void onUpdate() override;
	};



	/**
	@brief NAP Openframeworks spline selection component
	**/
	class OFSplineSelectionComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)

	public:
		// Default constructor
		OFSplineSelectionComponent();

		// Attributes
		Attribute<SplineType>	mSplineType		{ this, "SplineType",	SplineType::Circle };
		Attribute<float>		mSplineSize		{ this, "SplineSize",	20.0f };
		Attribute<int>			mSplineCount	{ this, "SplineCount",	100 };

		// Create slot
		NSLOT(mTypeChangedSlot,  const SplineType&, SplineTypeChanged)
		NSLOT(mSizeChangedSlot,  const float&,		SplineSizeChanged)
		NSLOT(mCountChangedSlot, const int&,		SplineCountChanged)

	private:
		// Slot functions
		void SplineTypeChanged(const SplineType& inType)		{ CreateAndUpdateSpline(); }
		void SplineSizeChanged(const float& inSize)				{ CreateAndUpdateSpline(); }
		void SplineCountChanged(const int& inCount)				{ CreateAndUpdateSpline(); }

		// Creates a new spline and sets it
		void CreateAndUpdateSpline();
	};



	/**
	@brief OFSpline Color Component
	**/
	class OFSplineColorComponent : public OFUpdatableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFUpdatableComponent)

	public:
		//@name Default constructor
		OFSplineColorComponent();

		//@name Update function
		virtual void	onUpdate() override;

		//@name Utility
		void			AddColor(const ofFloatColor& inColor)	{ mColors.getValueRef().emplace_back(inColor); }
		void			ClearColors()							{ mColors.getValueRef().clear(); }
		ofFloatColor&	GetColor(uint inIndex)					{ assert(inIndex < GetCount()); return mColors.getValueRef()[inIndex]; }
		uint			GetCount()								{ return mColors.getValue().size();  }

		//@name Attributes
		Attribute<float> mCycleSpeed =							{ this, "Cycle Speed", 0.0f };
		Attribute<float> mOffset =								{ this, "Offset", 0.0f };
		Attribute<std::vector<ofFloatColor>> mColors			{ this, "Colors", { 1.0f } };
		Attribute<float> mIntensity =							{ this, "Intensity", 1.0f };

	private:
		// Timer
		float								mPreviousTime;
		float								mTime = 0.0f;
	};

}

// RTTI Declarations
RTTI_DECLARE(nap::OFSplineComponent)
RTTI_DECLARE(nap::OFSplineSelectionComponent)
RTTI_DECLARE(nap::OFSplineColorComponent)
RTTI_DECLARE(nap::OFSplineUpdateGPUComponent)
