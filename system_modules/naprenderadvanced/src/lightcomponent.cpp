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

// nap::LightComponent run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponent)
	RTTI_PROPERTY("Enabled",			&nap::LightComponent::mEnabled,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CastShadows",		&nap::LightComponent::mCastShadows,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",				&nap::LightComponent::mColor,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Intensity",			&nap::LightComponent::mIntensity,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowStrength",		&nap::LightComponent::mShadowStrength,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::LightComponentInstance run time class definition
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
		mColor = mResource->mColor;
		mIntensity = mResource->mIntensity;

		// Fetch transform
		mTransform = &getEntityInstance()->getComponent<TransformComponentInstance>();

		// Reserve memory for light parameter list
		// It may never be reallocated outside of init()
		if (mParameterList.capacity() == 0)
			mParameterList.reserve(uniform::light::defaultMemberCount);

		// Create default parameters
		registerLightUniformMember<ParameterRGBColorFloat, RGBColorFloat>(uniform::light::color, nullptr, mResource->mColor);
		registerLightUniformMember<ParameterFloat, float>(uniform::light::intensity, nullptr, mResource->mIntensity);

		if (mIsShadowEnabled)
		{
			if (!errorState.check(getShadowCamera() != nullptr, "%s: Shadows are enabled while no shadow camera is set", mID.c_str()))
				return false;
		}

        // Register with service
        if (mIsEnabled)
        {
            auto *service = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
            assert(service != nullptr);
            service->registerLightComponent(*this);

            mIsRegistered = true;
        }
		return true;
	}


	Parameter* LightComponentInstance::getLightUniform(const std::string& memberName)
	{
		const auto it = mUniformDataMap.find(memberName);
		if (it != mUniformDataMap.end())
			return it->second;

		assert(false);
		return nullptr;
	}


	void LightComponentInstance::removeLightComponent()
	{
        if (mIsRegistered)
        {
            auto *service = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
            assert(service != nullptr);
            service->removeLightComponent(*this);

            mIsRegistered = false;
        }
	}
}
