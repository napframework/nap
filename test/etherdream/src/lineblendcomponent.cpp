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

		mSelectorOne->mIndexChanged.connect(mSelectionChangedSlot);
		mSelectorTwo->mIndexChanged.connect(mSelectionChangedSlot);

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

		nap::PolyLine& line_one = mSelectorOne->getLine();
		nap::PolyLine& line_two = mSelectorTwo->getLine();

		std::vector<glm::vec3>& pos_data = mTarget->getPositionAttr().getData();
		std::vector<glm::vec3>& nor_data = mTarget->getNormalAttr().getData();
		std::vector<glm::vec3>& uvs_data = mTarget->getUvAttr().getData();

		// Update distances when run for first time
		if (mDistancesLineOne.empty())
			cacheVertexAttributes(*mSelectorOne);
		if (mDistancesLineTwo.empty())
			cacheVertexAttributes(*mSelectorTwo);

		int vertex_count = mTarget->getMeshInstance().getNumVertices();
		assert(vertex_count > 1);
		for (int i = 0; i < vertex_count; i++)
		{
			// Interpolate position
			pos_data[i] = math::lerp<glm::vec3>(mPositionsLineOne[i], mPoistionsLineTwo[i], b_value);

			// Interpolate normal
			nor_data[i] = math::lerp<glm::vec3>(mNormalsLineOne[i], mNormalsLineTwo[i], b_value);

			// Interpolate uvs
			uvs_data[i] = math::lerp<glm::vec3>(mUvsLineOne[i], mUVsLineTwo[i], b_value);
		}

		nap::utility::ErrorState error;
		if (!(mTarget->getMeshInstance().update(error)))
		{
			nap::Logger::warn(error.toString().c_str());
		}
	}


	bool LineBlendComponentInstance::isClosed() const
	{
		return mSelectorOne->getLine().isClosed() && mSelectorTwo->getLine().isClosed();
	}


	void LineBlendComponentInstance::cacheVertexAttributes(const LineSelectionComponentInstance& selector)
	{
		// Update distance sample map
		std::map<float, int>& distances = &selector == mSelectorOne ? mDistancesLineOne : mDistancesLineTwo;
		selector.getLine().getDistances(distances);

		// Get vectors to cache
		std::vector<glm::vec3>& positions = &selector == mSelectorOne ? mPositionsLineOne : mPoistionsLineTwo;
		std::vector<glm::vec3>& normals = &selector == mSelectorOne ? mNormalsLineOne : mNormalsLineTwo;
		std::vector<glm::vec3>& uvs = &selector == mSelectorOne ? mUvsLineOne : mUVsLineTwo;

		const nap::PolyLine& current_line = selector.getLine();

		// Get attributes to interpolate from
		const nap::Vec3VertexAttribute& pos_attr = current_line.getPositionAttr();
		const nap::Vec3VertexAttribute& nor_attr = current_line.getNormalAttr();
		const nap::Vec3VertexAttribute& uvs_attr = current_line.getUvAttr();

		// Get number of vertices to set
		int vertex_count = mTarget->getMeshInstance().getNumVertices();
		assert(vertex_count > 1);
		
		positions.resize(vertex_count);
		normals.resize(vertex_count);
		uvs.resize(vertex_count);

		// Value used to blend
		float inc = 1.0f / static_cast<float>(vertex_count - 1);

		for (int i = 0; i < vertex_count; i++)
		{
			// Calculate inc value along line
			float c_inc = static_cast<float>(i) * inc;

			// Get interpolated data
			current_line.getValue<glm::vec3>(distances, pos_attr, c_inc, positions[i]);
			current_line.getValue<glm::vec3>(distances, nor_attr, c_inc, normals[i]);
			current_line.getValue<glm::vec3>(distances, uvs_attr, c_inc, uvs[i]);
		}
	}


	void LineBlendComponentInstance::onSelectionChanged(const LineSelectionComponentInstance& selectionComponent)
	{
		cacheVertexAttributes(selectionComponent);
	}
}