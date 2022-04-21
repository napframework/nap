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
	RTTI_PROPERTY("LightPosition",				&nap::LightComponent::mLightPositionParam,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LightIntensity",				&nap::LightComponent::mLightIntensityParam,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("TargetTransform",			&nap::LightComponent::mTargetTransform,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("EyeCamera",					&nap::LightComponent::mEyeCamera,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowCameraOrthographic",	&nap::LightComponent::mShadowCameraOrthographic,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowCameraPerspective",	&nap::LightComponent::mShadowCameraPerspective,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CameraType",					&nap::LightComponent::mCameraType,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableShadow",				&nap::LightComponent::mEnableShadow,				nap::rtti::EPropertyMetaData::Default)
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
		constexpr const char* vertUboStruct		= "VERTUBO";
		constexpr const char* fragUboStruct		= "FRAGUBO";
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

	static EntityInstance* getRootEntityRecursive(EntityInstance* instance)
	{
		if (instance->getParent() != nullptr)
			return getRootEntityRecursive(instance->getParent());
		return instance;
	}


	static bool hasDeclaration(MaterialInstance& materialInstance, std::string name)
	{
		if (materialInstance.findUniform(name) != nullptr)
			return true;

		// Find the declaration in the shader (if we can't find it, it's not a name that actually exists in the shader, which is an error).
		const ShaderVariableStructDeclaration* declaration = nullptr;
		const std::vector<BufferObjectDeclaration>& ubo_declarations = materialInstance.getMaterial().getShader().getUBODeclarations();
		for (const auto& ubo_declaration : ubo_declarations)
		{
			if (ubo_declaration.mName == name)
			{
				declaration = &ubo_declaration;
				return true;
			}
		}
		return false;
	}

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

		// Ensure shadow camera component is available id shadows are enabled
		if (mResource->mEnableShadow)
		{
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
		}

		mCameraEnabled = mResource->mEyeCamera != nullptr;

		auto* entity_instance = getEntityInstance();
		mTransform = &entity_instance->getComponent<TransformComponentInstance>();

		auto* root_entity = getRootEntityRecursive(entity_instance);
		mCachedRenderComponents.clear();
		root_entity->getComponentsOfTypeRecursive<RenderableMeshComponentInstance>(mCachedRenderComponents);

		return true;
	}


	void LightComponentInstance::update(double deltaTime)
	{
		// Calculate new light direction
		const glm::vec3 light_position = mResource->mLightPositionParam->mValue;
		const glm::vec3 light_direction = glm::normalize(mTargetTransformComponent->getTranslate() - light_position);

		// Get light transformation and update
		mTransform->setTranslate(light_position);
		mTransform->setRotate(glm::rotation({ 0.0f, 0.0f, -1.0f }, light_direction));

		// Only directional light with a orthocamera is currently supported
		if (mResource->mEnableShadow)
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

		glm::vec3 camera_location;
		if (mCameraEnabled)
		{
			camera_location = mEyeCameraComponent->getEntityInstance()->getComponent<TransformComponentInstance>().getTranslate();
		}

		for (auto* render_comp : mCachedRenderComponents)
		{
			if (hasDeclaration(render_comp->getMaterialInstance(), uniform::vertUboStruct))
			{
				UniformStructInstance* ubo_struct = render_comp->getMaterialInstance().getOrCreateUniform(uniform::vertUboStruct);
				if (ubo_struct != nullptr)
				{
					ubo_struct->getOrCreateUniform<UniformMat4Instance>(uniform::lightSpaceMatrix)->setValue(mLightViewProjection);
					ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightPosition)->setValue(light_position);
				}
			}

			if (hasDeclaration(render_comp->getMaterialInstance(), uniform::fragUboStruct))
			{
				UniformStructInstance* ubo_struct = render_comp->getMaterialInstance().getOrCreateUniform(uniform::fragUboStruct);
				if (ubo_struct != nullptr)
				{
					ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::cameraLocation)->setValue(camera_location);
					ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightPosition)->setValue(light_position);
					ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightDirection)->setValue(light_direction);
					ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::lightColor)->setValue(mResource->mLightColorParam->getValue().toVec3());
					ubo_struct->getOrCreateUniform<UniformFloatInstance>(uniform::lightIntensity)->setValue(mResource->mLightIntensityParam->mValue);
				}
			}
		}
	}


	CameraComponentInstance* LightComponentInstance::getShadowCamera() const
	{
		if (mResource->mEnableShadow && mShadowCameraOrthographic != nullptr)
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
}
