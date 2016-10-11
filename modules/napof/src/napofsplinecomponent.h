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
		Attribute<SplineType>		mSplineType =	{ this, "Type",	SplineType::Circle };
		NumericAttribute<float>		mSplineSize =	{ this, "Size",	500.0f, 0.0f, 1000.0f };
		NumericAttribute<int>		mSplineCount =	{ this, "PointCount",	500, 100, 1000 };
		NumericAttribute<int>		mSplineIndex =	{ this, "Index", 0, 0, (int)(SplineType::Max) };

		// Create slot
		NSLOT(mTypeChangedSlot,  const SplineType&, SplineTypeChanged)
		NSLOT(mSizeChangedSlot,  const float&,		SplineSizeChanged)
		NSLOT(mCountChangedSlot, const int&,		SplineCountChanged)
		NSLOT(mIndexChangedSlot, const int&, 		SplineIndexChanged)

	private:
		// Slot functions
		void SplineTypeChanged(const SplineType& inType)		{ CreateAndUpdateSpline(); }
		void SplineSizeChanged(const float& inSize)				{ CreateAndUpdateSpline(); }
		void SplineCountChanged(const int& inCount)				{ CreateAndUpdateSpline(); }
		void SplineIndexChanged(const int& idx)					{ mSplineType.setValue((SplineType)(idx)); }

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
		SignalAttribute mReload								{ this, "Reload" };
		SignalAttribute mBrowse								{ this, "Browse" };
		Attribute<std::string> mFile						{ this, "File" };
		NumericAttribute<int> mSplineCount					{ this, "PointCount", 500, 100, 1000 };

		// SLOTS
		NSLOT(mFileChangedSlot, const std::string&, fileChanged)
		NSLOT(mCountChangedSlot, const int&, countChanged)
		NSLOT(mReloadCalled, const SignalAttribute&, reloadCalled)
		NSLOT(mBrowseCalled, const SignalAttribute&, browseCalled)

	private:
		void fileChanged(const std::string& file)			{ createAndUpdateSpline(); }
		void countChanged(const int& count)					{ createAndUpdateSpline(); }
		void reloadCalled(const SignalAttribute&)			{ createAndUpdateSpline(); }
		void browseCalled(const SignalAttribute&);

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

		//@name Attributes
		Attribute<bool>			mStep =							{ this, "Stepped", false };
		Attribute<bool>			mClose =						{ this, "Looping", false };
		NumericAttribute<float> mCycleSpeed =					{ this, "CycleSpeed", 0.0f, 0.0f, 1.0f };
		NumericAttribute<float> mOffset =						{ this, "Offset", 0.0f, 0.0f, 1.0f };
		NumericAttribute<float> mIntensity =					{ this, "Intensity", 1.0f, 0.0f, 1.0f };
		NumericAttribute<float> mFrequency =					{ this, "Frequency", 1.0f, 1.0f, 100.0f };
		NumericAttribute<float> mFrequencyPower =				{ this, "Power", 1.0f, 0.0f, 5.0f};
		Attribute<ofFloatColor> mColorOne =						{ this, "ColorOne", { 1.0f } };
		Attribute<ofFloatColor> mColorTwo =						{ this, "ColorTwo", { 1.0f } };

	private:
		// Timer
		float									mPreviousTime;
		float									mTime = 0.0f;
		ComponentDependency<OFSplineComponent>	mSpline			{ this };

		ofFloatColor& getColorForIdx(int idx);
	};

}

// RTTI Declarations
RTTI_DECLARE(nap::OFSplineComponent)
RTTI_DECLARE(nap::OFSplineSelectionComponent)
RTTI_DECLARE(nap::OFSplineColorComponent)
RTTI_DECLARE(nap::OFSplineUpdateGPUComponent)
RTTI_DECLARE(nap::OFSplineFromFileComponent)
