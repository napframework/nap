#include "lightcomponent.h"

// Local includes
#include "renderadvancedservice.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <transformcomponent.h>
#include <uniforminstance.h>
#include <materialinstance.h>
#include <renderablemeshcomponent.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// nap::LightComponent run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponent)
	RTTI_PROPERTY("Enable Shadows", &nap::LightComponent::mEnableShadows, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::LightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

// nap::DirectionalLightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::DirectionalLightComponent)
	RTTI_PROPERTY("Color", &nap::DirectionalLightComponent::mColor, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Intensity", &nap::DirectionalLightComponent::mIntensity, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowCamera", &nap::DirectionalLightComponent::mShadowCamera, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::DirectionalLightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::DirectionalLightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// LightComponent
	//////////////////////////////////////////////////////////////////////////

	void LightComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// LightComponentInstance
	//////////////////////////////////////////////////////////////////////////

	bool LightComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<LightComponent>();
		mIsShadowEnabled = mResource->mEnableShadows;

		// Fetch transform
		mTransform = &getEntityInstance()->getComponent<TransformComponentInstance>();

		if (mIsShadowEnabled)
		{
			if(!errorState.check(getShadowCamera() != nullptr, "%s: Shadows are enabled No shadow camera set", mID.c_str()))
				return false;
		}

		// Register with service
		auto* service = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
		assert(service != nullptr);
		service->registerLightComponent(*this);

		return true;
	}


	void LightComponentInstance::update(double deltaTime)
	{
		// Calculate new light direction
		const glm::vec3 light_position = math::extractPosition(mTransform->getGlobalTransform());
		const glm::vec3 light_direction = -glm::normalize(mTransform->getGlobalTransform()[2]);
	}


	LightComponentInstance::~LightComponentInstance()
	{
		auto* service = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
		assert(service != nullptr);
		service->removeLightComponent(*this);
	}


	//////////////////////////////////////////////////////////////////////////
	// DirectionalLightComponent
	//////////////////////////////////////////////////////////////////////////

	bool DirectionalLightComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!LightComponentInstance::init(errorState))
			return false;

		auto* resource = static_cast<DirectionalLightComponent*>(mResource);
		mColor = resource->mColor;
		mIntensity = resource->mIntensity;

		return true;
	}


	void DirectionalLightComponentInstance::update(double deltaTime)
	{

	}
}
