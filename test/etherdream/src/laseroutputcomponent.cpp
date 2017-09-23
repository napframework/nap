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
	RTTI_PROPERTY("Line",		&nap::LaserOutputComponent::mLine,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Properties",	&nap::LaserOutputComponent::mProperties,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LaserOutputComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
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
	bool LaserOutputComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
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
		nap::PolyLine* line = rtti_cast<nap::PolyLine>(&(mLine->getMesh()));
		assert(line != nullptr);

		// Get attribute data
		VertexAttribute<glm::vec3>& position_data = line->getPositionAttr();
		VertexAttribute<glm::vec4>& color_data = line->getColorAttr();

		// Get xfrom
		nap::TransformComponentInstance& xform = mLine->getEntityInstance()->getComponent<nap::TransformComponentInstance>();

		// Populate the laser buffer
		nap::TransformComponentInstance& laser_xform = this->getEntityInstance()->getComponent<nap::TransformComponentInstance>();
		populateLaserBuffer(position_data.getData(), color_data.getData(), laser_xform.getGlobalTransform(), xform.getGlobalTransform());
	}


	void LaserOutputComponentInstance::populateLaserBuffer(const std::vector<glm::vec3>& verts, const std::vector<glm::vec4>& colors, const glm::mat4x4& laserXform, const glm::mat4x4& lineXform)
	{
		assert(verts.size() == colors.size());

		// Create new list of points to copy
		mPoints.clear();

		// Get frustrum dimensions and transform
		glm::vec2 frustrum = { 0.0f, 0.0f };

		// Get frustrum dimensions
		float fr_width	= mProperties.mFrustrum.x;
		float fr_height = mProperties.mFrustrum.y;

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
			mPoints[i].R = sEtherInterpolateColor(cc.r * cc.a);
			mPoints[i].G = sEtherInterpolateColor(cc.g * cc.a);
			mPoints[i].B = sEtherInterpolateColor(cc.b * cc.a);
			mPoints[i].I = sEtherInterpolateColor(cc.a);
		}

		mDac->setPoints(mPoints);
	}

}
