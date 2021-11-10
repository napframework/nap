/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "laseroutputcomponent.h"

// External Includes
#include <entity.h>
#include <nap/logger.h>
#include <mathutils.h>
#include <lineutils.h>
#include <etherdreaminterface.h>

using namespace nap::math;

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_STRUCT(nap::LaserOutputProperties)
	RTTI_PROPERTY("Frustum",		&nap::LaserOutputProperties::mFrustum,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlipVertical",	&nap::LaserOutputProperties::mFlipVertical,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal", &nap::LaserOutputProperties::mFlipHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Framerate",		&nap::LaserOutputProperties::mFrameRate,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GapThreshold",	&nap::LaserOutputProperties::mGapThreshold,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::LaserOutputComponent)
	RTTI_PROPERTY("Dac",			&nap::LaserOutputComponent::mDac,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Line",			&nap::LaserOutputComponent::mLine,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Transform",		&nap::LaserOutputComponent::mLineTransform,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Properties",		&nap::LaserOutputComponent::mProperties,		nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserOutputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


// Interpolate etherdream value between min / max values
static int16_t sEtherInterpolate(float value, float min, float max, bool flip)
{
	return (int16_t)fit<float>(value, min, max, 
		flip ? nap::EtherDreamInterface::etherMax() : nap::EtherDreamInterface::etherMin(),
		flip ? nap::EtherDreamInterface::etherMin() : nap::EtherDreamInterface::etherMax());
}


// Interpolate normalized color channel to min / max laser value 
static int16_t sEtherInterpolateColor(float inValue)
{
	return static_cast<int16_t>(lerp<float>(0.0f, nap::EtherDreamInterface::etherMax(), inValue));
}


// bi-cubic ease in / out utility that is used to close the gap between disconnected begin / end points
static float gapEaseInOut(float p)
{
	if (p < 0.5) 
		return 4 * p * p * p;
	else
	{
		float f = ((2 * p) - 2);
		return 0.5 * f * f * f + 1;
	}
}


//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool LaserOutputComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over link to the DAC
		LaserOutputComponent* output_resource = getComponent<LaserOutputComponent>();
		mDac = output_resource->mDac.get();

		// Copy over mesh
		mLine = getComponent<LaserOutputComponent>()->mLine.get();

		// Copy over properties
		mProperties = output_resource->mProperties;
		return true;
	}


	void LaserOutputComponentInstance::update(double deltaTime)
	{
		// Send the polyline to the dac based on the location of the laser and the location of the line
		populateLaserBuffer(*mLine, mLineTransform->getGlobalTransform());
	}


	void LaserOutputComponentInstance::populateLaserBuffer(const PolyLine& line, const glm::mat4x4& lineXform)
	{
		const Vec3VertexAttribute& vert_attr = line.getPositionAttr();
		const Vec4VertexAttribute& colr_attr = line.getColorAttr();

		// Get attribute data
		const std::vector<glm::vec3>& verts = line.getPositionAttr().getData();
		const std::vector<glm::vec4>& colors = line.getColorAttr().getData();

		assert(verts.size() == colors.size());
		assert(verts.size() > 1);

		// Get the total amount of points per frame that this laser is allowed to draw and resize buffer
		int ppf = static_cast<int>(static_cast<float>(mDac->mPointRate) / static_cast<float>(mProperties.mFrameRate));
		mVerts.resize(ppf);
		mColors.resize(ppf);

		// Check the distance between the first and end point
		const glm::vec3& first_vert = verts.front();
		const glm::vec3& last_vert = verts.back();

		// Get gap distance
		float gap_dist = glm::distance(first_vert, last_vert);

		// Initially the line contains the max number of points
		int line_points = ppf;
		
		// If there is a certain distance between the first and last vertex of the line we can redistribute the points
		// otherwise the entire line is taken up by the initial line and there is no gap, ie: all points belong to the actual line
		bool has_gap = gap_dist > mProperties.mGapThreshold && !(line.isClosed());
		if (has_gap)
		{
			// Get complete line distance
			std::map<float, int> distance_map;
			float line_dist = line.getDistances(distance_map);

			// Calculate gap to line point distribution
			// The bigger the gap between the first and last vertex, the more gap points will be distributed
			float lg_ratio = line_dist / gap_dist;
			float lg_s = static_cast<float>(ppf + 1) / (lg_ratio + 1.0);
			line_points = math::min(static_cast<int>(lg_ratio * lg_s), ppf);
		}

		// Populate re-interpolated line buffer based on line length
		float line_inc = 1.0f / static_cast<float>(std::max(line_points - (has_gap ? 1 : 0),1));
		for (int i = 0; i < line_points; i++)
		{
			float sample_idx = line_inc * static_cast<float>(i);
			line.getValue<glm::vec3>(vert_attr, sample_idx, mVerts[i]);
			line.getValue<glm::vec4>(colr_attr, sample_idx, mColors[i]);
		}

		// Now populate the part in between the line (gap)
		// The incremental value is lower because we don't want to include the last and first point of the 
		// actual line that was drawn. To move more points toward the end and start points we use
		// a cubic easy in out
		float gap_inc = 1.0f / static_cast<float>((ppf-line_points) + 1);
		for (int i = line_points; i < ppf; i++)
		{
			float lerp_v = gapEaseInOut(gap_inc * static_cast<float>((i - line_points) + 1));
			mVerts[i]  = math::lerp<glm::vec3>(last_vert, first_vert, lerp_v);
			mColors[i] = { 0.0f, 0.0f,0.0f,0.0f };
		}

		// Get frustum dimensions and transform
		glm::vec2 frustum = { 0.0f, 0.0f };

		// Get frustum dimensions
		float fr_width	= mProperties.mFrustum.x;
		float fr_height = mProperties.mFrustum.y;

		// Calculate frustrum bounds
		glm::vec2 min_bounds(frustum.x - (fr_width / 2.0f), frustum.y - (fr_height / 2.0f));
		glm::vec2 max_bounds(frustum.x + (fr_width / 2.0f), frustum.y + (fr_height / 2.0f));

		// Reserve all the points
		mPoints.resize(mVerts.size());

		// Go over the curves and set data
		for (uint32 i = 0; i < mVerts.size(); i++)
		{
			// Transform vertex
			glm::vec3 cv = lineXform * glm::vec4(mVerts[i], 1.0f);

			// Get color
			const glm::vec4& cc = mColors[i];

			// Sets
			mPoints[i].X = sEtherInterpolate(cv.x, min_bounds.x, max_bounds.x, false);
			mPoints[i].Y = sEtherInterpolate(cv.y, min_bounds.y, max_bounds.y, false);
			mPoints[i].R = sEtherInterpolateColor(cc.r * cc.a);
			mPoints[i].G = sEtherInterpolateColor(cc.g * cc.a);
			mPoints[i].B = sEtherInterpolateColor(cc.b * cc.a);
			mPoints[i].I = sEtherInterpolateColor(cc.a);
		}

		mDac->setPoints(mPoints);
	}
}
