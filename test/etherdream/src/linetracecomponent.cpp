#include "linetracecomponent.h"

#include <nap/entity.h>
#include <mathutils.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::TraceProperties)
	RTTI_PROPERTY("Offset",	&nap::TraceProperties::mOffset,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",	&nap::TraceProperties::mSpeed,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length",	&nap::TraceProperties::mLength,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::LineTraceComponent)
	RTTI_PROPERTY("Properties",			&nap::LineTraceComponent::mProperties,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BlendComponent",		&nap::LineTraceComponent::mBlendComponent,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Target",				&nap::LineTraceComponent::mTargetLine,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineTraceComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{

	bool LineTraceComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Copy properties
		mProperties.mSpeed  = math::clamp<float>(getComponent<LineTraceComponent>()->mProperties.mSpeed, 0.0f,1.0f);
		mProperties.mLength = math::clamp<float>(getComponent<LineTraceComponent>()->mProperties.mLength, 0.0f, 1.0f);
		mProperties.mSpeed  = getComponent<LineTraceComponent>()->mProperties.mSpeed;

		// Set lines
		mBlendComponent = getComponent<LineTraceComponent>()->mBlendComponent.get();
		mTarget = getComponent<LineTraceComponent>()->mTargetLine.get();

		return true;
	}


	void LineTraceComponentInstance::update(double deltaTime)
	{
		// Get line to trace
		const nap::PolyLine& source = mBlendComponent->getLine();

		// Calculate start position
		mCurrentTime += (deltaTime * mProperties.mSpeed);

		// Prep value for sin
		float start_pos = fmod((mCurrentTime + mProperties.mOffset), 1.0f);

		// Ensure our target mesh has more than 1 vertex (ie, is a line)
		int target_vert_count = mTarget->getMeshInstance().getNumVertices();
		assert(target_vert_count > 1);

		// Get incremental value
		float inc = mProperties.mLength / (target_vert_count - 1);

		// Get source line
		const nap::PolyLine& source_line = mBlendComponent->getLine();

		std::vector<glm::vec3>& pos_attr_data = mTarget->getPositionAttr().getData();
		std::vector<glm::vec3>& nor_attr_data = mTarget->getNormalAttr().getData();
		std::vector<glm::vec4>& col_attr_data = mTarget->getColorAttr().getData();
		std::vector<glm::vec3>& uvs_attr_data = mTarget->getUvAttr().getData();

		// Blend values
		float current_pos = start_pos;
		for (int i = 0; i < target_vert_count; i++)
		{
			// Wrap position
			current_pos = fmod(current_pos, 1.0f);

			// Set position
			source_line.getPosition(current_pos, pos_attr_data[i]);

			// Set color
			source_line.getColor(current_pos, col_attr_data[i]);

			// Set normal
			source_line.getNormal(current_pos, nor_attr_data[i]);

			// Set uv
			source_line.getUv(current_pos, uvs_attr_data[i]);

			// Increment position
			current_pos += inc;
		}

		nap::utility::ErrorState error;
		if (!(mTarget->getMeshInstance().update(error)))
		{
			nap::Logger::warn(error.toString().c_str());
		}
	}

}