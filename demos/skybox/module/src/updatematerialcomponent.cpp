#include "updatematerialcomponent.h"

// External Includes
#include <entity.h>
#include <renderablemeshcomponent.h>
#include <blinnphongcolorshader.h>
#include <uniformupdate.h>

// nap::UpdateMaterialComponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateMaterialComponent)
	RTTI_PROPERTY("Color",		&nap::UpdateMaterialComponent::mColor,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Fresnel",	&nap::UpdateMaterialComponent::mFresnel,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::UpdateMaterialComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateMaterialComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{ 
	void UpdateMaterialComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(RenderableMeshComponent));
	}


	bool UpdateMaterialComponentInstance::init(utility::ErrorState& errorState)
	{
		auto* resource = getComponent<UpdateMaterialComponent>();
		assert(resource != nullptr);

		auto* mesh_comp = &getEntityInstance()->getComponent<RenderableMeshComponentInstance>();
		assert(mesh_comp != nullptr);

		auto* uni = mesh_comp->getMaterialInstance().getOrCreateUniform("UBO");
		if (!errorState.check(uni != nullptr, "Missing uniform struct with name `UBO`"))
			return false;

		auto* color = uni->getOrCreateUniform<UniformVec3Instance>("color");
		if (!errorState.check(color != nullptr, "Missing uniform vec3 member with name `color`"))
			return false;
		registerUniformUpdate(*color, *resource->mColor);

		auto* fresnel = uni->getOrCreateUniform<UniformVec2Instance>("fresnel");
		if (!errorState.check(fresnel != nullptr, "Missing uniform vec2 member with name `fresnel`"))
			return false;
		registerUniformUpdate(*fresnel, *resource->mFresnel);

		return true;
	}
}
