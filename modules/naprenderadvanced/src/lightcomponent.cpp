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
	RTTI_PROPERTY("Color",				&nap::LightComponent::mColor,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Intensity",			&nap::LightComponent::mIntensity,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Enable Shadows",		&nap::LightComponent::mEnableShadows,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::LightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponentInstance)
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

		// Create default parameters
		registerLightUniformMember(uniform::light::color, mResource->mColor.get());
		registerLightUniformMember(uniform::light::intensity, mResource->mIntensity.get());

		if (mIsShadowEnabled)
		{
			if(!errorState.check(getShadowCamera() != nullptr, "%s: Shadows are enabled No shadow camera set", mID.c_str()))
				return false;
		}

		// Flags [type : 8bit][equation : 8bit][shadow : 1bit][padding : 15bit]
		mLightFlags = static_cast<uint32>(getLightType());
		//mLightFlags |= static_cast<uint32>(getLightEquation()) << 8U;
		mLightFlags |= static_cast<uint32>(isShadowEnabled()) << 16U;

		// Register with service
		auto* service = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
		assert(service != nullptr);
		service->registerLightComponent(*this);

		return true;
	}


	void LightComponentInstance::registerLightUniformMember(const std::string& memberName, Parameter* parameter)
	{
		const auto it = mUniformDataMap.insert({ memberName, parameter });
		assert(it.second);
	}


	Parameter* LightComponentInstance::getLightUniform(const std::string& memberName)
	{
		const auto it = mUniformDataMap.find(memberName);
		if (it != mUniformDataMap.end())
			return it->second;

		assert(false);
		return nullptr;
	}


	LightComponentInstance::~LightComponentInstance()
	{
		auto* service = getEntityInstance()->getCore()->getService<RenderAdvancedService>();
		assert(service != nullptr);
		service->removeLightComponent(*this);
	}
}
