#pragma once

// RTTI Includes
#include <nap.h>

// NAP OF Includes
#include <napofrendercomponent.h>
#include <napofupdatecomponent.h>
#include <napofattributes.h>
#include <utils/ofVec2i.h>

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
		NumericAttribute<float>		mSpeed		{ this, "Speed", 0.1f, 0.0f, 1.0f };
		NumericAttribute<float>		mLength		{ this, "Size", 0.1f, 0.0f, 1.0f  };
		NumericAttribute<int>		mCount		{ this, "PointCount", 500, 100, 1000   };
		Attribute<bool>				mDrawDot	{ this, "DrawDot", true };
		NumericAttribute<float>		mOffset		{ this, "Offset", 0.0f, 0.0f, 1.0f };
		Attribute<ofVec2i>			mEdgeOffset	{ this, "EdgeOffset", { 10, 3 } };
		NumericAttribute<float>		mLineWidth	{ this, "LineWidth", 1.0f, 0.0f, 1.0f };

		// Getters
		const SplineVertexData&	getVerts()	{ return mTraceSpline.GetVertexDataRef(); }
		const SplineColorData&	getColors() { return mTraceSpline.GetColorDataRef(); }

		// Create slot
		NSLOT(mCountChanged, AttributeBase&, UpdateTraceBuffer)

	private:
		// Spline to populate and draw
		NSpline						mTraceSpline;

		// Updates the trace spline buffer
		void						UpdateTraceBuffer(AttributeBase& attr);

		// Time variables
		float						mTime = 0.0f;
		float						mPreviousTime;
	};
}

RTTI_DECLARE(nap::OFTraceComponent)
