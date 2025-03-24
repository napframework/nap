/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "lightcomponent.h"
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

RTTI_BEGIN_ENUM(nap::ELightType)
	RTTI_ENUM_VALUE(nap::ELightType::Custom,		"Custom"),
	RTTI_ENUM_VALUE(nap::ELightType::Directional,	"Directional"),
	RTTI_ENUM_VALUE(nap::ELightType::Point,			"Point"),
	RTTI_ENUM_VALUE(nap::ELightType::Spot,			"Spot")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EShadowMapType)
	RTTI_ENUM_VALUE(nap::EShadowMapType::Quad,		"Quad"),
	RTTI_ENUM_VALUE(nap::EShadowMapType::Cube,		"Cube")
RTTI_END_ENUM

RTTI_BEGIN_STRUCT(nap::LightComponent::Locator)
	RTTI_PROPERTY("LineWidth",	&nap::LightComponent::Locator::mLineWidth,	nap::rtti::EPropertyMetaData::Default, "Locator line width")
	RTTI_PROPERTY("GnomonSize", &nap::LightComponent::Locator::mGnomonSize, nap::rtti::EPropertyMetaData::Default, "Locator size")
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponent)
	RTTI_PROPERTY("Enabled",			&nap::LightComponent::mEnabled,				nap::rtti::EPropertyMetaData::Default, "If the light is enabled")
	RTTI_PROPERTY("CastShadows",		&nap::LightComponent::mCastShadows,			nap::rtti::EPropertyMetaData::Default, "If the light casts shadows")
	RTTI_PROPERTY("Color",				&nap::LightComponent::mColor,				nap::rtti::EPropertyMetaData::Default, "The color of the light source")
	RTTI_PROPERTY("Intensity",			&nap::LightComponent::mIntensity,			nap::rtti::EPropertyMetaData::Default, "The intensity of the light source")
	RTTI_PROPERTY("ShadowStrength",		&nap::LightComponent::mShadowStrength,		nap::rtti::EPropertyMetaData::Default, "The amount of light the shadow consumes")
	RTTI_PROPERTY("ShadowSpread",		&nap::LightComponent::mShadowSpread,		nap::rtti::EPropertyMetaData::Default, "The spread radius of the shadow samples")
	RTTI_PROPERTY("Locator",			&nap::LightComponent::mLocator,				nap::rtti::EPropertyMetaData::Default, "Locator options")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponentInstance)
	RTTI_PROPERTY(nap::uniform::light::color,		&nap::LightComponentInstance::mColor,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY(nap::uniform::light::intensity,	&nap::LightComponentInstance::mIntensity,	nap::rtti::EPropertyMetaData::Default)
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
		mIsEnabled = mResource->mEnabled;
		mIsShadowEnabled = mResource->mCastShadows;
		mShadowStrength = mResource->mShadowStrength;
		mShadowSpread = mResource->mShadowSpread;
		mColor = mResource->mColor;
		mIntensity = mResource->mIntensity;

		// Fetch transform
		mTransform = &getEntityInstance()->getComponent<TransformComponentInstance>();
		if(!errorState.check(mTransform != nullptr, "Missing %s", RTTI_OF(nap::TransformComponent).get_name().data()))
			return false;

		// Fetch render advanced service
		mService = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
		assert(mService != nullptr);

		// Reserve memory for light uniform property list
		mUniformList.reserve(uniform::light::defaultMemberCount);

		// Register uniform light properties
		registerUniformLightProperty(nap::uniform::light::color);
		registerUniformLightProperty(nap::uniform::light::intensity);

        // Register with service
        mService->registerLightComponent(*this);
		return true;
	}


	void LightComponentInstance::registerUniformLightProperty(const std::string& memberName)
	{
		auto uniform_prop = this->get_type().get_property(memberName);
		assert(uniform_prop.is_valid());
		assert(std::find(mUniformList.begin(), mUniformList.end(), uniform_prop) == mUniformList.end());
		mUniformList.emplace_back(std::move(uniform_prop));
	}


	void LightComponentInstance::onDestroy()
	{
		if (mSpawnedCamera != nullptr)
			mService->destroy(mSpawnedCamera);
		mService->removeLightComponent(*this);
	}


	nap::SpawnedEntityInstance LightComponentInstance::spawnShadowCamera(const nap::Entity& entity, nap::utility::ErrorState& error)
	{
		// Ensure there's a camera
		assert(mSpawnedCamera == nullptr);
		if (!error.check(entity.findComponent(RTTI_OF(nap::CameraComponent)) != nullptr,
			"Missing %s", RTTI_OF(nap::CameraComponent).get_name().data()))
			return mSpawnedCamera;

		// Ensure the camera has an origin
		auto origin_comp = entity.findComponent(RTTI_OF(nap::RenderGnomonComponent));
		if (!error.check(origin_comp != nullptr,
			"Missing %s", RTTI_OF(nap::RenderGnomonComponent).get_name().data()))
			return mSpawnedCamera;

		// Spawn it and return
		mSpawnedCamera = mService->spawn(entity, error);
		return mSpawnedCamera;
	}
}
