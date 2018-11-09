#include "renderabletextcomponent.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>

// nap::renderabletextcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderableTextComponent)
	RTTI_PROPERTY("Text",				&nap::RenderableTextComponent::mText,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Font",				&nap::RenderableTextComponent::mFont,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableTextComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::renderabletextcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableTextComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderableTextComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	bool RenderableTextComponentInstance::init(utility::ErrorState& errorState)
	{
		return true;
	}


	void RenderableTextComponentInstance::update(double deltaTime)
	{

	}


	void RenderableTextComponentInstance::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{

	}

}