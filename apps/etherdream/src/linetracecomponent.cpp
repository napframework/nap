#include "linetracecomponent.h"

#include <nap/entity.h>
#include <mathutils.h>
#include <nap/logger.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>

RTTI_BEGIN_CLASS(nap::TraceProperties)
	RTTI_PROPERTY("Offset",				&nap::TraceProperties::mOffset,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OffsetSmoothTime",	&nap::TraceProperties::mOffsetSmoothTime,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Speed",				&nap::TraceProperties::mSpeed,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SpeedSmoothTime",	&nap::TraceProperties::mSpeedSmoothTime,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length",				&nap::TraceProperties::mLength,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LengthSmoothTime",	&nap::TraceProperties::mLengthSmoothTime,	nap::rtti::EPropertyMetaData::Required)
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
		mTarget = getComponent<LineTraceComponent>()->mTargetLine.get();

		// Create the trace visualize components
		LineTraceComponent* resource = getComponent<LineTraceComponent>();		
		ResourceManager& resource_manager = *getEntityInstance()->getCore()->getResourceManager();

		if (!errorState.check(getEntityInstance()->getChildren().size() == 2, "Expected one child"))
			return false;

		EntityInstance* start_vis_entity = getEntityInstance()->getChildren()[0];
		EntityInstance* end_vis_entity = getEntityInstance()->getChildren()[1];

		mStartXform = start_vis_entity->findComponent<nap::TransformComponentInstance>();
		if (!errorState.check(mStartXform != nullptr, "Trace visualizer has no transform component"))
			return false;

		auto start_render = start_vis_entity->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(start_render != nullptr, "Trace visualizer has no renderable component"))
			return false;

		UniformVec3& uniform = start_render->getMaterialInstance().getOrCreateUniform<UniformVec3>("mColor");
		uniform.setValue(glm::vec3(1.0f, 0.0f, 0.0f));

		mEndXform = end_vis_entity->findComponent<nap::TransformComponentInstance>();
		if (!errorState.check(mEndXform != nullptr, "Trace visualizer has no transform component"))
			return false;

		auto end_render = end_vis_entity->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(end_render != nullptr, "Trace visualizer has no renderable component"))
			return false;

		UniformVec3& euniform = end_render->getMaterialInstance().getOrCreateUniform<UniformVec3>("mColor");
		euniform.setValue(glm::vec3(0.0f, 1.0f, 0.0f));

		// Set smooth timing values
		mLengthSmoother.mSmoothTime = getComponent<LineTraceComponent>()->mProperties.mLengthSmoothTime;
		mLengthSmoother.setValue(mProperties.mLength);

		mOffsetSmoother.mSmoothTime = getComponent<LineTraceComponent>()->mProperties.mOffsetSmoothTime;
		mOffsetSmoother.setValue(mProperties.mOffset);

		mSpeedSmoother.mSmoothTime = getComponent<LineTraceComponent>()->mProperties.mSpeedSmoothTime;
		mSpeedSmoother.setValue(mProperties.mSpeed);

		return true;
	}


	void LineTraceComponentInstance::update(double deltaTime)
	{
		mOffsetSmoother.update(mProperties.mOffset, deltaTime);
		mSpeedSmoother.update(mProperties.mSpeed, deltaTime);
		mLengthSmoother.update(mProperties.mLength, deltaTime);

		// Get line to trace
		nap::PolyLine& source = mBlendComponent->getLine();

		// Calculate start position
		mCurrentTime += (deltaTime * mSpeedSmoother.getValue());

		// Prep value for sin
		float start_pos = fmod((mCurrentTime + mOffsetSmoother.getValue()), 1.0f);

		// Ensure our target mesh has more than 1 vertex (ie, is a line)
		int target_vert_count = mTarget->getMeshInstance().getNumVertices();
		assert(target_vert_count > 1);

		// Get incremental value
		float inc = mLengthSmoother.getValue() / (target_vert_count - 1);

		// Get source line
		nap::PolyLine& source_line = mBlendComponent->getLine();

		std::vector<glm::vec3>& pos_attr_data = mTarget->getPositionAttr().getData();
		std::vector<glm::vec4>& col_attr_data = mTarget->getColorAttr().getData();

		Vec3VertexAttribute& source_pos_attr = source_line.getPositionAttr();
		Vec4VertexAttribute& source_col_attr = source_line.getColorAttr();

		// Get vertex distances of source
		std::map<float, int> distances;
		float line_length = source_line.getDistances(distances);

		// Blend values
		float current_pos = start_pos;
		for (int i = 0; i < target_vert_count; i++)
		{
			// Wrap position
			current_pos = fmod(current_pos, 1.0f);

			// Set position
			source_line.getValue<glm::vec3>(distances, source_pos_attr, current_pos, pos_attr_data[i]);

			// Set color
			source_line.getValue<glm::vec4>(distances, source_col_attr, current_pos, col_attr_data[i]);

			// Increment position
			current_pos += inc;
		}

		nap::utility::ErrorState error;
		if (!(mTarget->getMeshInstance().update(error)))
		{
			nap::Logger::warn(error.toString().c_str());
		}

		// Update visualizer location
		mStartXform->setTranslate(pos_attr_data.front());
		mEndXform->setTranslate(pos_attr_data.back());
	}


	void LineTraceComponentInstance::reset()
	{
		mCurrentTime = 0.0f;
	}

	
	void LineTraceComponentInstance::setPolyLine(nap::PolyLine& line)
	{
		mTarget = &line;
	}

}