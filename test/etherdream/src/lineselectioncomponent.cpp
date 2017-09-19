#include "lineselectioncomponent.h"
#include <mathutils.h>
#include <nap/entity.h>
#include <nap/logger.h>
#include <transformcomponent.h>

RTTI_BEGIN_CLASS(nap::LineSelectionComponent)
	RTTI_PROPERTY("Lines", &nap::LineSelectionComponent::mLines, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index", &nap::LineSelectionComponent::mIndex, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineSelectionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineSelectionComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// Get renderable mesh component
		mMeshComponentInstance = getEntityInstance()->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(mMeshComponentInstance != nullptr, "missing renderable component"))
			return false;

		// Copy over the list of lines to selection from
		for (auto& line : getComponent<LineSelectionComponent>()->mLines)
		{
			RenderableMesh mesh = mMeshComponentInstance->createRenderableMesh(*line, errorState);
			if (!errorState.check(mesh.isValid(), "Attempt to use a mesh in LineSelectionComponent which does not match the material in the RenderableMeshComponent"))
				return false;

			mLines.push_back(mesh);
		}

		// Ensure there are lines to choose from
		if (!(errorState.check(mLines.size() > 0, "No lines to select from")))
			return false;

		// Get renderable mesh component
		mMeshComponentInstance = getEntityInstance()->findComponent<RenderableMeshComponentInstance>();
		if (!errorState.check(mMeshComponentInstance != nullptr, "missing renderable component"))
			return false;

		// Make sure index is in range
		verifyIndex(getComponent<LineSelectionComponent>()->mIndex);

		return true;
	}


	const nap::PolyLine& LineSelectionComponentInstance::getLine() const
	{
		return(static_cast<const nap::PolyLine&>(mLines[mIndex].getMesh()));
	}


	void LineSelectionComponentInstance::setIndex(int index)
	{
		verifyIndex(index);
		mMeshComponentInstance->setMesh(mLines[mIndex]);
	}


	void LineSelectionComponentInstance::verifyIndex(int index)
	{
		// Make sure the index is in range
		mIndex = math::clamp<int>(index, 0, mLines.size() - 1);
	}
}