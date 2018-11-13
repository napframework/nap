#include "renderable3dtextcomponent.h"

// External Includes
#include <entity.h>

// nap::Renderable3DTextComponent run time class definition 
RTTI_BEGIN_CLASS(nap::Renderable3DTextComponent)
	// Put additional properties here
RTTI_END_CLASS

// nap::Renderable3DTextComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Renderable3DTextComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void Renderable3DTextComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	bool Renderable3DTextComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableTextComponentInstance::init(errorState))
			return false;

		if (!errorState.check(hasTransform(), "%s doesn't have a transform component", getEntityInstance()->mID.c_str()))
			return false;
		return true;
	}


	void Renderable3DTextComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		assert(hasTransform());
		const glm::mat4x4 model_matrix = getTransform()->getGlobalTransform();
		Renderable3DTextComponentInstance::draw(viewMatrix, projectionMatrix, model_matrix);
	}

}