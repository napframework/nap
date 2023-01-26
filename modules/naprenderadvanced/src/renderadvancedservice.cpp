/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderadvancedservice.h"

// External Includes
#include <renderservice.h>
#include <rendercomponent.h>
#include <renderablemeshcomponent.h>
#include <nap/core.h>
#include <rtti/factory.h>

RTTI_BEGIN_CLASS(nap::RenderAdvancedServiceConfiguration)
	RTTI_PROPERTY("ShadowMapSize", &nap::RenderAdvancedServiceConfiguration::mShadowMapSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderAdvancedService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS
	
namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Get the light uniform struct
	 */
	static UniformStructInstance* getLightStruct(MaterialInstance& materialInstance, utility::ErrorState& errorState)
	{
		// Ensure the member can be created
		auto* ubo_struct = materialInstance.getMaterial().findUniform(uniform::lightStruct);
		if (!errorState.check(ubo_struct != nullptr,
			"The shader bound to material instance '%s' with shader '%s' requires an UBO with name '%s'",
			materialInstance.getMaterial().mID.c_str(), materialInstance.getMaterial().getShader().getDisplayName().c_str(), uniform::lightStruct))
			return nullptr;

		// Safely create and set the member
		return materialInstance.getOrCreateUniform(uniform::lightStruct);
	}


	/**
	 * Get the light uniform struct
	 */
	//static UniformStructInstance* verifyLightStruct(UniformStructInstance* lightStruct, utility::ErrorState& errorState)
	//{
	//	auto* light_array = lightStruct->getOrCreateUniform<UniformStructArrayInstance>(uniform::light::lights);
	//	if (!errorState.check(light_array != nullptr,
	//		"The shader bound to material instance '%s' with shader '%s' requires an UBO with name '%s'",
	//		materialInstance.getMaterial().mID.c_str(), materialInstance.getMaterial().getShader().getDisplayName().c_str(), uniform::lightStruct))
	//		return nullptr;

	//	// Safely create and set the member
	//	return materialInstance.getOrCreateUniform(uniform::lightStruct);
	//}


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
	static bool updateLight(UniformStructInstance* lightStruct, uint index, T value, utility::ErrorState& errorState)
	{
		auto* ubo_member = lightStruct->findUniform<TypedUniformValueArrayInstance<T>>(memberName);
		if (!errorState.check(ubo_member != nullptr,
			"UBO '%s' requires a member of type '%s' with name '%s' in material instance '%s' with shader '%s'",
			ubo_struct->getDeclaration().mName.c_str(), RTTI_OF(T).get_name().to_string().c_str(), memberName.c_str(), materialInstance.getMaterial().mID.c_str(), materialInstance.getMaterial().getShader().getDisplayName().c_str()))
			return false;

		// Safely create and set the member
		auto* instance = materialInstance.getOrCreateUniform(uniform::lightStruct)->getOrCreateUniform<TypedUniformValueArrayInstance<T>>(memberName);
		instance->setValue(value, index);

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// Render Advanced Service Configuration
	//////////////////////////////////////////////////////////////////////////

	rtti::TypeInfo RenderAdvancedServiceConfiguration::getServiceType() const
	{
		return RTTI_OF(RenderAdvancedService);
	}


	//////////////////////////////////////////////////////////////////////////
	// Render Advanced Service
	//////////////////////////////////////////////////////////////////////////

	void RenderAdvancedService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	bool RenderAdvancedService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void RenderAdvancedService::update(double deltaTime)
	{
		//
	}


	void RenderAdvancedService::renderShadows(const std::vector<RenderableComponentInstance*>& comps)
	{
		auto* render_service = getCore().getService<RenderService>();
		assert(render_service != nullptr);

		// Evaluate registered light components
		for (const auto& light : mLightComponents)
		{
			auto* shadow_camera = light->getShadowCamera();
			if (shadow_camera != nullptr)
			{
				// One shadow render pass per light
				mShadowMapTarget->beginRendering();
				render_service->renderObjects(*mShadowMapTarget, *shadow_camera, comps);
				mShadowMapTarget->endRendering();
			}
		}

		// Filter render components
		std::vector<RenderableMeshComponentInstance*> filtered_mesh_comps;
		for (auto& comp : comps)
		{
			// Ensure the component is visible and had a material instance
			if (comp->isVisible() && comp->get_type().is_derived_from(RTTI_OF(RenderableMeshComponentInstance)))
			{
				// Ensure the shader interface supports lights
				auto* mesh_comp = static_cast<RenderableMeshComponentInstance*>(comp);
				if (mesh_comp->getMaterialInstance().getMaterial().findUniform(uniform::lightStruct) != nullptr)
					filtered_mesh_comps.emplace_back(mesh_comp);
			}
		}

		// Update materials for color pass
		utility::ErrorState error_state; // TODO: resolve nicely later
		for (auto& mesh_comp : filtered_mesh_comps)
		{
			// Verify shader interface for lights
			auto* light_struct = mesh_comp->getMaterialInstance().getOrCreateUniform(uniform::lightStruct);
			if (light_struct == nullptr)
				continue;

			auto* light_array = light_struct->getOrCreateUniform<UniformStructArrayInstance>(uniform::light::lights);
			if (light_array == nullptr)
				continue;

			auto* light_count = light_struct->getOrCreateUniform<UniformUIntInstance>(uniform::light::count);
			if (light_count == nullptr)
				continue;

			uint count = 0;
			for (const auto& light : mLightComponents)
			{
				if (count >= light_array->getElements().size())
					break;

				// Fetch element
				auto& light_element = light_array->getElement(count);

				// Calculate light variables
				const glm::vec3 light_position = math::extractPosition(light->getTransform().getGlobalTransform());
				const glm::vec3 light_direction = -glm::normalize(light->getTransform().getGlobalTransform()[2]);

				// TODO: All this stuff should somehow be handled by the light implementation, we cant account for every possible
				// shader interface for specific light types.
				if (light->get_type().is_derived_from<DirectionalLightComponentInstance>())
				{
					light_element.getOrCreateUniform<UniformVec3Instance>(uniform::light::directional::direction)->setValue(light_direction);

					auto* directional_light = static_cast<DirectionalLightComponentInstance*>(light);
					light_element.getOrCreateUniform<UniformVec3Instance>(uniform::light::directional::color)->setValue(directional_light->mColor);
					light_element.getOrCreateUniform<UniformFloatInstance>(uniform::light::directional::intensity)->setValue(directional_light->mIntensity);
				}

				if (light->isShadowEnabled())
				{
					const auto light_view_projection = light->getShadowCamera()->getProjectionMatrix() * light->getShadowCamera()->getViewMatrix();
					light_element.getOrCreateUniform<UniformMat4Instance>(uniform::light::lightViewProjection)->setValue(light_view_projection);

					auto shadow_sampler = mesh_comp->getMaterialInstance().getOrCreateSampler(sampler::light::shadowMap);
					if (shadow_sampler->get_type().is_derived_from(RTTI_OF(Sampler2DInstance)))
					{
						auto* shadow_sampler_2d = static_cast<Sampler2DInstance*>(shadow_sampler);
						shadow_sampler_2d->setTexture(*mShadowMapTexture);
					}
				}
				++count;
			}
			light_count->setValue(count);
		}
	}


	void RenderAdvancedService::preShutdown()
	{
		mShadowMapTexture.reset();
		mShadowMapTarget.reset();
		mLightComponents.clear();
	}


	void RenderAdvancedService::registerLightComponent(LightComponentInstance& light)
	{
		mLightComponents.emplace_back(&light);

		// Generate shadow map
		if (mShadowMapTarget == nullptr)
		{
			auto* configuration = getConfiguration<RenderAdvancedServiceConfiguration>();

			auto shadow_map = std::make_unique<DepthRenderTexture2D>(getCore());
			shadow_map->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
			shadow_map->mWidth = configuration->mShadowMapSize;
			shadow_map->mHeight = configuration->mShadowMapSize;
			shadow_map->mUsage = ETextureUsage::Static;
			shadow_map->mFormat = DepthRenderTexture2D::EDepthFormat::D16;
			shadow_map->mColorSpace = EColorSpace::Linear;
			shadow_map->mClearValue = 1.0f;
			shadow_map->mFill = true;
			mShadowMapTexture = std::move(shadow_map);

			utility::ErrorState error_state;
			if (!mShadowMapTexture->init(error_state))
			{
				assert(false);
				return;
			}

			auto shadow_target = std::make_unique<DepthRenderTarget>(getCore());
			shadow_target->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
			shadow_target->mClearValue = 1.0f;
			mShadowMapTarget = std::move(shadow_target);
			mShadowMapTarget->mDepthTexture = mShadowMapTexture.get();

			if (!mShadowMapTarget->init(error_state))
			{
				assert(false);
				return;
			}
		}
	}
}
