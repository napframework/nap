// Local Includes
#include <napoftracecomponent.h>

// OF Includes
#include <napofsplinecomponent.h>
#include <utils/ofUtils.h>
#include <utils/nofUtils.h>

// STD Includes
#include <cmath>

static const ofFloatColor sTraceEdgeColor(0.0f, 0.0f, 0.0f, 0.0f);

namespace nap
{
	OFTraceComponent::OFTraceComponent()
	{
		// Set first sample time
		mPreviousTime = ofGetElapsedTimef();

		// Connect signals / slots
		mCount.connectToValue(mCountChanged);

		// Create buffer
		UpdateTraceBuffer(mCount.getValue());

		// Disable alpha blending
		mAlphaBlending.setValue(false);
	}


	void OFTraceComponent::onDraw()
	{
		// Get time difference and store
		float current_time = ofGetElapsedTimef();
		float diff_time = current_time - mPreviousTime;
		mPreviousTime = current_time;

		// Get the spline component
		OFSplineComponent* spline_component = getParent()->getComponent<OFSplineComponent>();
		if (spline_component == nullptr)
		{
			Logger::warn("No spline component found, can't draw tracer");
			return;
		}

		// Get the spline
		NSpline& spline = spline_component->mSpline.getValueRef();
		if (spline.GetPointCount() < 2)
			return;

		// Update time
		mTime += (diff_time * mSpeed.getValue());
		float sample_loc = fmod(mTime + mOffset.getValueRef(), 1.0f);

		// Get update step over spline
		float step = mLength.getValue() / float(mCount.getValue());

		uint32 trace_count  = uint32(mCount.getValue());
		uint32 spline_count = uint32(spline.GetPointCount());

		// Update all the trace points
		for (uint32 i = 0; i < trace_count; i++)
		{
			// Set vertex location
			float lookup = sample_loc + (float(i) * step);
			lookup = fmod(lookup, 1.0f);
			mTraceSpline.GetVertex(i) = spline.GetPointAtPercent(lookup);

			// Get the index for the color buffers to interpolate
			float f_idx = spline.GetIndexAtPercent(lookup);

			// Get the two colors closest to that point
			uint32 min_point_if = gMin<int>((uint32)f_idx, spline_count-1);
			uint32 max_point_if = min_point_if + 1 >= spline_count ? 0 : min_point_if + 1;

			// Check if the point we're sampling is close to the edge of the spline
			bool near_start = min_point_if < mEdgeOffset.getValueRef().x;
			bool near_end	= min_point_if >= spline_count - mEdgeOffset.getValueRef().y;
			
			// Discard close to edge points
			if ((near_start || near_end) && !spline.IsClosed())
			{
				mTraceSpline.GetColor(i) = sTraceEdgeColor;
				continue;
			}

			// Get colors of min / max point
			const ofFloatColor& min_color = spline.GetColor(min_point_if);
			const ofFloatColor& max_color = spline.GetColor(max_point_if);

			// Interpolate based on intermediate value
			float lerp_value = f_idx - float(uint32(f_idx));
			gMixFloatColor(min_color, max_color, lerp_value, mTraceSpline.GetColor(i));
		}

		// Upload to gpu
		mTraceSpline.UpdateVBO(NSpline::DataType::VERTEX);

		// Draw trace spline
		ofSetColor(255, 255, 255, 255);
		mTraceSpline.SetLineWidth(mLineWidth.getValue());
		mTraceSpline.Draw();

		// Draw a dot over time
		ofSetColor(255, 255, 0, 255);
		if (mDrawDot.getValue())
		{
			ofCircle(mTraceSpline.GetVertex(0), 4.0f);
			ofCircle(mTraceSpline.GetVertex(mTraceSpline.GetPointCount() - 1), 4.0f);
		}
	}


	/**
	@brief Updates the trace buffer (NSpline) based on the points in the 
	**/
	void OFTraceComponent::UpdateTraceBuffer(const int& inCount)
	{
		// Create poly line
		ofPolyline line;

		// Create vertices
		std::vector<ofPoint> vertices;
		vertices.resize(inCount);
		line.addVertices(vertices);

		// Set line as buffer, created additional buffers on the fly
		mTraceSpline.SetPolyLine(line);
	}
}

RTTI_DEFINE(nap::OFTraceComponent)
