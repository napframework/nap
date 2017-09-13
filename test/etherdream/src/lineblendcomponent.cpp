#include "lineblendcomponent.h"
#include <nap/entity.h>
#include <mathutils.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::LineBlendComponent)
	RTTI_PROPERTY("SelectionComponentOne",	&nap::LineBlendComponent::mSelectionComponentOne,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("SelectionComponentTwo",	&nap::LineBlendComponent::mSelectionComponentTwo,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Target",					&nap::LineBlendComponent::mTarget,					nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BlendValue",				&nap::LineBlendComponent::mBlendValue,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("BlendSpeed",				&nap::LineBlendComponent::mBlendSpeed,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineBlendComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineBlendComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Set the two selector objects
		mSelectorOne = getComponent<LineBlendComponent>()->mSelectionComponentOne.get();
		mSelectorTwo = getComponent<LineBlendComponent>()->mSelectionComponentTwo.get();

		// Set blend value
		mBlendValue = getComponent<LineBlendComponent>()->mBlendValue;
		mBlendSpeed = getComponent<LineBlendComponent>()->mBlendSpeed;

		// Copy line
		mTarget = getComponent<LineBlendComponent>()->mTarget.get();

		return true;
	}


	void LineBlendComponentInstance::update(double deltaTime)
	{
		// Calculate current blend based on speed
		mCurrentTime += (deltaTime * mBlendSpeed);

		// Prep value for sin
		float b_value = (mBlendValue*M_PI) + mCurrentTime;

		// Get normalized blend value starting from 0
		b_value = (sin(b_value-(M_PI / 2))+1) / 2.0f;

		const nap::PolyLine& line_one = mSelectorOne->getLine();
		const nap::PolyLine& line_two = mSelectorTwo->getLine();

		std::vector<glm::vec3>& pos_data = mTarget->getPositionAttr().getData();
		std::vector<glm::vec3>& nor_data = mTarget->getNormalAttr().getData();
		std::vector<glm::vec3>& uvs_data = mTarget->getUvAttr().getData();
		std::vector<glm::vec4>& col_data = mTarget->getColorAttr().getData();

		glm::vec3 line_pos_one, line_pos_two;
		glm::vec3 line_nor_one, line_nor_two;
		glm::vec3 line_uvs_one, line_uvs_two;
		glm::vec4 line_col_one, line_col_two;

		int vertex_count = mTarget->getMeshInstance().getNumVertices();
		assert(vertex_count > 1);
		float inc = 1.0f / static_cast<float>(vertex_count - 1);
		for (int i = 0; i < vertex_count; i++)
		{
			// Calculate inc value along line
			float c_inc = static_cast<float>(i) * inc;
			
			// Get position data for both lines
			line_one.getPosition(c_inc, line_pos_one);
			line_two.getPosition(c_inc, line_pos_two);

			// Interpolate position
			pos_data[i] = math::lerp<glm::vec3>(line_pos_one, line_pos_two, b_value);

			line_one.getNormal(c_inc, line_nor_one);
			line_two.getNormal(c_inc, line_nor_two);

			// Interpolate normal
			nor_data[i] = math::lerp<glm::vec3>(line_nor_one, line_nor_two, b_value);

			line_one.getColor(c_inc, line_col_one);
			line_two.getColor(c_inc, line_col_two);

			// Interpolate color
			col_data[i] = math::lerp<glm::vec4>(line_col_one, line_col_two, b_value);

			line_one.getUv(c_inc, line_uvs_one);
			line_two.getUv(c_inc, line_uvs_two);

			uvs_data[i] = math::lerp<glm::vec3>(line_uvs_one, line_uvs_two, b_value);
		}

		nap::utility::ErrorState error;
		if (!(mTarget->getMeshInstance().update(error)))
		{
			nap::Logger::warn(error.toString().c_str());
		}
	}

}