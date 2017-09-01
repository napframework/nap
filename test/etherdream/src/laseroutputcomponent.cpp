#include "lasershapecomponent.h"
#include "laseroutputcomponent.h"

#include <nap/entity.h>
#include <nap/logger.h>
#include <mathutils.h>

using namespace nap::math;

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::LaserOutputProperties)
	RTTI_PROPERTY("Frustrum",		&nap::LaserOutputProperties::mFrustrum,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FlipVertical",	&nap::LaserOutputProperties::mFlipVertical,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipHorizontal", &nap::LaserOutputProperties::mFlipHorizontal,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LaserOutputComponent)
	RTTI_PROPERTY("Dac",		&nap::LaserOutputComponent::mDac,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Properties",	&nap::LaserOutputComponent::mProperties,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserOutputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

static float etherMinValueFloat = static_cast<float>(nap::etherMinValue);
static float ehterMaxValueFloat = static_cast<float>(nap::etherMaxValue);

// Interpolate etherdream value between min / max values
static int16_t sEtherInterpolate(float value, float min, float max, bool flip)
{
	return (int16_t)fit<float>(value, min, max, 
		flip ? ehterMaxValueFloat : etherMinValueFloat, 
		flip ? etherMinValueFloat : ehterMaxValueFloat);
}


static int16_t sEtherInterpolateColor(float inValue)
{
	return static_cast<int16_t>(lerp<float>(0.0f, ehterMaxValueFloat, inValue));
}

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	bool LaserOutputComponentInstance::init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy over link to the DAC
		LaserOutputComponent* output_resource = rtti_cast<LaserOutputComponent>(resource.get());
		mDac = output_resource->mDac;

		// Copy over properties
		mProperties = output_resource->mProperties;
		return true;
	}


	// Sample line and convert to laser
	void LaserOutputComponentInstance::setLine(PolyLine& line, const glm::mat4x4& xform)
	{
		TypedVertexAttribute<glm::vec3>* position_data = line.getPositionData();
		assert(position_data != nullptr);
		TypedVertexAttribute<glm::vec4>* color_data = line.getColorData();
		assert(color_data != nullptr);

		// Populate the laser buffer
		nap::TransformComponentInstance& laser_xform = this->getEntity()->getComponent<nap::TransformComponentInstance>();
		populateLaserBuffer(position_data->getValues(), color_data->getValues(), laser_xform.getGlobalTransform(), xform);
	}


	void LaserOutputComponentInstance::update(double deltaTime)
	{
		mDac->setPoints(mPoints);
	}


	void LaserOutputComponentInstance::populateLaserBuffer(std::vector<glm::vec3>& verts, const std::vector<glm::vec4>& colors, const glm::mat4x4& laserXform, const glm::mat4x4& lineXform)
	{
		assert(verts.size() == colors.size());

		// Create new list of points to copy
		mPoints.clear();

		// Get frustrum dimensions and transform
		glm::vec2 frustrum = { 0.0f, 0.0f };

		// Get frustrum dimensions
		float fr_width	= mProperties.mFrustrum.x;
		float fr_height = mProperties.mFrustrum.y;

		// Get frustrum bounds
		float div_h = fr_height / 2.0f;
		float div_w = fr_width / 2.0f;

		glm::vec2 min_bounds(frustrum.x - (fr_width / 2.0f), frustrum.y - (fr_height / 2.0f));
		glm::vec2 max_bounds(frustrum.x + (fr_width / 2.0f), frustrum.y + (fr_height / 2.0f));

		// Reserve all the points
		mPoints.resize(verts.size());

		// Sample xform for object movement
		const glm::mat4x4& global_xform = lineXform;

		// Go over the curves and set data
		for (uint32 i = 0; i < verts.size(); i++)
		{
			// Transform vertex
			glm::vec3 cv = lineXform * glm::vec4(verts[i], 1.0f);

			// Get color
			const glm::vec4& cc = colors[i];

			// Sets
			mPoints[i].X = sEtherInterpolate(cv.x, min_bounds.x, max_bounds.x, false);
			mPoints[i].Y = sEtherInterpolate(cv.y, min_bounds.y, max_bounds.y, false);
			mPoints[i].R = sEtherInterpolateColor(cc.r);
			mPoints[i].G = sEtherInterpolateColor(cc.g);
			mPoints[i].B = sEtherInterpolateColor(cc.b);
			mPoints[i].I = sEtherInterpolateColor(cc.a);
		}

		mDac->setPoints(mPoints);
	}

}