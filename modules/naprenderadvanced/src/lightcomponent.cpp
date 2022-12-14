#include "lightcomponent.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <uniforminstance.h>
#include <materialinstance.h>
#include <renderablemeshcomponent.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// nap::LightComponent run time class definition 
RTTI_BEGIN_CLASS(nap::LightComponent)
	RTTI_PROPERTY("LightColor",					&nap::LightComponent::mLightColorParam,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LightIntensity",				&nap::LightComponent::mLightIntensityParam,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EyeCamera",					&nap::LightComponent::mEyeCamera,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowCameraOrthographic",	&nap::LightComponent::mShadowCameraOrthographic,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowCameraPerspective",	&nap::LightComponent::mShadowCameraPerspective,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraType",					&nap::LightComponent::mCameraType,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RenderComponents",			&nap::LightComponent::mRenderComponents,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthRenderTarget",			&nap::LightComponent::mDepthRenderTarget,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::LightComponentInstance run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LightComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Constant
	//////////////////////////////////////////////////////////////////////////

	namespace uniform
	{
		constexpr const char* VERTUBO			= "VERTUBO";
		constexpr const char* FRAGUBO			= "FRAGUBO";
		constexpr const char* lightPosition		= "lightPosition";
		constexpr const char* lightDirection	= "lightDirection";
		constexpr const char* lightColor		= "lightColor";
		constexpr const char* lightIntensity	= "lightIntensity";
		constexpr const char* lightSpaceMatrix	= "lightSpaceMatrix";
		constexpr const char* cameraLocation	= "cameraLocation";
	}


	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Ensures the value member is created without asserting. Reports a verbose error message if this is not the case.
	 *
	 * @param materialInstance
	 * @param uboName
	 * @param memberName
	 * @param errorState
	 * @tparam the uniform instance type to create 
	 * @return if the member was created
	 */
	template<typename T>
	static bool ensureValueMember(MaterialInstance& materialInstance, const std::string& uboName, const std::string& memberName, T value, utility::ErrorState& errorState)
	{
		// Ensure the member can be created
		auto* ubo_struct = materialInstance.getMaterial().findUniform(uboName);
		if (!errorState.check(ubo_struct != nullptr,
			"The shader bound to material instance '%s' with shader '%s' requires an UBO with name '%s'",
			materialInstance.getMaterial().mID.c_str(), materialInstance.getMaterial().getShader().getDisplayName().c_str(), uboName.c_str()))
			return false;

		auto* member = ubo_struct->findUniform<TypedUniformValueInstance<T>>(memberName);
		if (!errorState.check(member != nullptr,
			"UBO '%s' requires a member of type '%s' with name '%s' in material instance '%s' with shader '%s'",
			ubo_struct->getDeclaration().mName.c_str(), RTTI_OF(T).get_name().to_string().c_str(), memberName.c_str(), materialInstance.getMaterial().mID.c_str(), materialInstance.getMaterial().getShader().getDisplayName().c_str()))
			return false;

		// Safely create and set the member
		materialInstance.getOrCreateUniform(uboName)->getOrCreateUniform<TypedUniformValueInstance<T>>(memberName)->setValue(value);

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// LightComponent
	//////////////////////////////////////////////////////////////////////////

	void LightComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
		components.emplace_back(RTTI_OF(RenderableMeshComponent));
	}


	//////////////////////////////////////////////////////////////////////////
	// LightComponentInstance
	//////////////////////////////////////////////////////////////////////////

	bool LightComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<LightComponent>();

		// Ensure shadow camera component is available id shadows are enabled
		switch (mResource->mCameraType)
		{
		case ECameraType::Perspective:
			if (!errorState.check(mShadowCameraPerspective != nullptr, "Property 'ShadowCameraPerspective' must not be NULL when 'EnableShadow' is enabled and 'CameraType' is 'Perspective'"))
				return false;
			break;
		case ECameraType::Orthographic:
			if (!errorState.check(mShadowCameraOrthographic != nullptr, "Property 'ShadowCameraOrthographic' must not be NULL when 'EnableShadow' is enabled and 'CameraType' is 'Orthographic'"))
				return false;
			break;
		default:
			assert(false);
			return false;
		}

		mCameraEnabled = mResource->mEyeCamera != nullptr;
		mShadowEnabled = mCameraEnabled && mResource->mDepthRenderTarget != nullptr;

		mTransform = &getEntityInstance()->getComponent<TransformComponentInstance>();

		// Ensure the material is compatible with nap::LightComponent
		for (auto& render_comp : mRenderComponents)
		{
			if (!ensureValueMember<glm::mat4>((*render_comp).getMaterialInstance(), uniform::VERTUBO, uniform::lightSpaceMatrix, mLightViewProjection, errorState))
				return false;

			if (!ensureValueMember<glm::vec3>((*render_comp).getMaterialInstance(), uniform::VERTUBO, uniform::lightPosition, mTransform->getTranslate(), errorState))
				return false;

			mResolvedRenderComponents.emplace_back(&(*render_comp));
		}

		return true;
	}


	void LightComponentInstance::update(double deltaTime)
	{
		// Calculate new light direction
		const glm::vec3 light_position = math::extractPosition(mTransform->getGlobalTransform());
		const glm::vec3 light_direction = -glm::normalize(mTransform->getGlobalTransform()[2]);

		// Only directional light with a orthocamera is currently supported
		if (mShadowEnabled)
		{
			// Calculate light view projection matrix
			switch (mResource->mCameraType)
			{
			case ECameraType::Perspective:
			{
				mLightViewProjection = mShadowCameraPerspective->getProjectionMatrix() * mShadowCameraPerspective->getViewMatrix();
				break;
			}
			case ECameraType::Orthographic:
			{
				mLightViewProjection = mShadowCameraOrthographic->getProjectionMatrix() * mShadowCameraOrthographic->getViewMatrix();
				break;
			}
			default:
				assert(false);
			}
		}

		const auto camera_transform = mCameraEnabled ? mEyeCameraComponent->getEntityInstance()->getComponent<TransformComponentInstance>().getGlobalTransform() : glm::mat4();
		const auto camera_location = math::extractPosition(camera_transform);
		for (auto& render_comp : mRenderComponents)
		{
			UniformStructInstance* vert_ubo_struct = (*render_comp).getMaterialInstance().getOrCreateUniform(uniform::VERTUBO);
			vert_ubo_struct->getOrCreateUniform<UniformMat4Instance>(uniform::lightSpaceMatrix)->setValue(mLightViewProjection);
			vert_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightPosition)->setValue(light_position);

			UniformStructInstance* frag_ubo_struct = (*render_comp).getMaterialInstance().getOrCreateUniform(uniform::FRAGUBO);
			frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::cameraLocation)->setValue(camera_location);
			frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightPosition)->setValue(light_position);
			frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightDirection)->setValue(light_direction);
			frag_ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightColor)->setValue(mResource->mLightColorParam->getValue().toVec3());
			frag_ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightIntensity)->setValue(mResource->mLightIntensityParam->mValue);
		}
	}


	bool LightComponentInstance::beginShadowPass()
	{
		if (!mShadowEnabled)
			return false;

		mResource->mDepthRenderTarget->beginRendering();
		return true;
	}


	void LightComponentInstance::endShadowPass()
	{
		assert(mShadowEnabled);
		mResource->mDepthRenderTarget->endRendering();
	}


	CameraComponentInstance* LightComponentInstance::getShadowCamera() const
	{
		if (mShadowEnabled && mShadowCameraOrthographic != nullptr)
		{
			switch (mResource->mCameraType)
			{
			case ECameraType::Perspective:
				return &mShadowCameraPerspective->getEntityInstance()->getComponent<PerspCameraComponentInstance>();
			case ECameraType::Orthographic:
				return &mShadowCameraOrthographic->getEntityInstance()->getComponent<OrthoCameraComponentInstance>();
			default:
				assert(false);
			}
		}
		return nullptr;
	}


	DepthRenderTarget& LightComponentInstance::getShadowTarget() const
	{
		assert(mShadowEnabled);
		return *mResource->mDepthRenderTarget;
	}
}
