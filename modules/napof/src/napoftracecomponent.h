#pragma once

// RTTI Includes
#include <rtti/rtti.h>

// NAP OF Includes
#include <napofrendercomponent.h>
#include <napofupdatecomponent.h>
#include <napofattributes.h>
#include <Utils/ofVec2i.h>

#include <nap/signalslot.h>

// OF Includes
#include <ofLog.h>
#include <ofVbo.h>
#include <ofPolyline.h>

namespace nap
{
	/**
	@brief NAP Openframeworks Spline Trace component
	**/
	class OFTraceComponent : public OFRenderableComponent
	{
		RTTI_ENABLE_DERIVED_FROM(OFRenderableComponent)

	public:
		// Default constructor
		OFTraceComponent();

		// Render
		virtual void onDraw() override;

		// Attributes
		Attribute<float>	mSpeed		{ this, "Speed", 0.1f };
		Attribute<float>	mLength		{ this, "Size", 0.1f };
		Attribute<int>		mCount		{ this, "Count", 100 };
		Attribute<bool>		mDrawDot	{ this, "DrawDot", true };
		Attribute<float>	mOffset		{ this, "Offset", 0.0f };
		Attribute<ofVec2i>	mEdgeOffset	{ this, "EdgeOffset", { 10, 3 } };
		Attribute<float>	mLineWidth	{ this, "LineWidth", 1.0f };

		// Getters
		const SplineVertexData&	getVerts()	{ return mTraceSpline.GetVertexDataRef(); }
		const SplineColorData&	getColors() { return mTraceSpline.GetColorDataRef(); }

		// Create slot
		NSLOT(mCountChanged, const int&, UpdateTraceBuffer)

	private:
		// Spline to populate and draw
		NSpline						mTraceSpline;

		// Updates the trace spline buffer
		void						UpdateTraceBuffer(const int& inCount);

		// Time variables
		float						mTime = 0.0f;
		float						mPreviousTime;
	};
}

RTTI_DECLARE(nap::OFTraceComponent)
