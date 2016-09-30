#pragma once

// Of Includes
#include <Spline/nofSpline.h>

// NAP Includes
#include <nap/component.h>
#include <nap/attribute.h>
#include <nap/signalslot.h>

#include <nap/configure.h>
#include <napofattributes.h>
#include <nap/componentdependency.h>

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
		Attribute<NSpline>		mSpline				{ this, "Spline" };				//< The spline the component manages
		Attribute<float>		mLineWidth			{ this, "LineWidth", 1.0f };

		// Utility
		void					SetFromPolyLine(const ofPolyline& inPolyline);
		uint					GetPointCount()		{ return mSpline.getValueRef().GetPointCount(); }
		bool					isClosed() const	{ return mSpline.getValue().IsClosed(); }

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

		// Spline dependency
		ComponentDependency<OFSplineComponent>	mSpline			{ this };
	};


	/**
	@brief NAP Openframeworks spline from file component

	Creates a spline from a svg file
	**/
	class OFSplineFromFileComponent : public Component
	{
		RTTI_ENABLE_DERIVED_FROM(Component)

	public:
		// Default constructor
		OFSplineFromFileComponent();

		// Attributes
		Attribute<std::string> mFile						{ this, "File" };
		Attribute<int> mSplineCount							{ this, "Count", 500 };

		// SLOTS
		NSLOT(mFileChangedSlot, const std::string&, fileChanged)
		NSLOT(mCountChangedSlot, const int&, countChanged)

	private:
		void fileChanged(const std::string& file)			{ createAndUpdateSpline(); }
		void countChanged(const int& count)					{ createAndUpdateSpline(); }

		// Creates and updates the spline based on file and count
		void createAndUpdateSpline();

		ComponentDependency<OFSplineComponent> mSpline		{ this };
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
		Attribute<float> mFrequency =							{ this, "Frequency", 1.0f };
		Attribute<float> mFrequencyPower =						{ this, "Power", 1.0f };
		Attribute<bool>	 mStep =								{ this, "Interpolated", false };

	private:
		// Timer
		float									mPreviousTime;
		float									mTime = 0.0f;
		ComponentDependency<OFSplineComponent>	mSpline			{ this };
	};

}

// RTTI Declarations
RTTI_DECLARE(nap::OFSplineComponent)
RTTI_DECLARE(nap::OFSplineSelectionComponent)
RTTI_DECLARE(nap::OFSplineColorComponent)
RTTI_DECLARE(nap::OFSplineUpdateGPUComponent)
RTTI_DECLARE(nap::OFSplineFromFileComponent)
