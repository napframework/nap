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

#include <parameter.h>
#include <parametervec.h>
#include <parameternumeric.h>
#include <parametermat.h>
#include <parametercolor.h>

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
		// Create and manage a shadow texture dummy for valid shadow samplers
		auto texture_dummy = std::make_unique<DepthRenderTexture2D>(getCore());
		texture_dummy->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(Texture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
		texture_dummy->mWidth = 16;
		texture_dummy->mHeight = 16;
		texture_dummy->mUsage = ETextureUsage::Static;
		texture_dummy->mFormat = DepthRenderTexture2D::EDepthFormat::D16;
		texture_dummy->mColorSpace = EColorSpace::Linear;
		texture_dummy->mClearValue = 1.0f;
		texture_dummy->mFill = true;
		mShadowTextureDummy = std::move(texture_dummy);

		if (!mShadowTextureDummy->init(errorState))
		{
			errorState.fail("%s: Failed to create shadow texture dummy", this->get_type().get_name().to_string().c_str());
			return false;
		}

		auto shadow_sampler_array = std::make_unique<Sampler2DArray>(sMaxShadowMapCount);
		shadow_sampler_array->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(Sampler2DArray).get_name().to_string().c_str(), math::generateUUID().c_str());
		shadow_sampler_array->mName = sampler::light::shadowMaps;

		// Copy pointers
		for (auto& tex : shadow_sampler_array->mTextures)
			tex = mShadowTextureDummy.get();

		shadow_sampler_array->mCompareMode = EDepthCompareMode::LessOrEqual;
		shadow_sampler_array->mBorderColor = EBorderColor::IntOpaqueBlack;
		shadow_sampler_array->mEnableCompare = true;
		mSamplerResource = std::move(shadow_sampler_array);

		if (!mSamplerResource->init(errorState))
		{
			errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
			return false;
		}

		return true;
	}


	void RenderAdvancedService::update(double deltaTime)
	{
		//
	}


	void RenderAdvancedService::renderShadows(const std::vector<RenderableComponentInstance*>& comps, bool updateMaterials)
	{
		auto* render_service = getCore().getService<RenderService>();
		assert(render_service != nullptr);

		// Evaluate registered light components
		for (const auto& light : mLightComponents)
		{
			// TODO: Only re-render shadow map when light transform or camera matrix has changed
			auto* shadow_camera = light->isShadowEnabled() ? light->getShadowCamera() : nullptr;
			if (shadow_camera != nullptr)
			{
				// One shadow render pass per light
				auto it = mLightDepthTargetMap.find(light);
				assert(it != mLightDepthTargetMap.end());

				auto& target = it->second;
				assert(target != nullptr);

				target->beginRendering();
				render_service->renderObjects(*target, *shadow_camera, comps);
				target->endRendering();
			}
		}

		if (updateMaterials)
		{
			utility::ErrorState error_state;
			if (!updateLightData(comps, error_state))
			{
				nap::Logger::error(error_state.toString());
				assert(false);
			}
		}
	}



	bool RenderAdvancedService::updateLightData(const std::vector<RenderableComponentInstance*>& comps, utility::ErrorState& errorState)
	{
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
				if (count >= light_array->getMaxNumElements())
					break;

				// Fetch element
				auto& light_element = light_array->getElement(count);

				// Light uniform defaults
				light_element.getOrCreateUniform<UniformVec3Instance>(uniform::light::origin)->setValue(light->getLightPosition());
				light_element.getOrCreateUniform<UniformVec3Instance>(uniform::light::direction)->setValue(light->getLightDirection());

				// Light uniform custom
				for (const auto& entry : light->mUniformDataMap)
				{
					const auto name = entry.first;
					if (name == uniform::light::origin || name == uniform::light::direction || name == uniform::light::lightViewProjection)
						continue;

					auto* struct_decl = static_cast<const ShaderVariableStructDeclaration*>(&light_element.getDeclaration());
					auto* member = struct_decl->findMember(name);
					if (!errorState.check(member != nullptr, "Missing uniform with name '%s'", name.c_str()))
						return false;

					if (member->get_type().is_derived_from(RTTI_OF(ShaderVariableValueDeclaration)))
					{
						auto* value_member = static_cast<const ShaderVariableValueDeclaration*>(member);
						if (value_member->mType == EShaderVariableValueType::Float)
						{
							auto* param = light->getLightUniform(name);
							if (!errorState.check(param != nullptr, "Unsupported member data type"))
								return false;

							light_element.getOrCreateUniform<UniformFloatInstance>(name)->setValue(static_cast<ParameterFloat*>(param)->mValue);
						}
						else if (value_member->mType == EShaderVariableValueType::Vec3)
						{
							auto* param = light->getLightUniform(name);
							if (!errorState.check(param != nullptr, "Unsupported member data type"))
								return false;

							if (param->get_type().is_derived_from(RTTI_OF(ParameterRGBColorFloat)))
							{
								light_element.getOrCreateUniform<UniformVec3Instance>(name)->setValue(static_cast<ParameterRGBColorFloat*>(param)->mValue.toVec3());
							}
							else if (param->get_type().is_derived_from(RTTI_OF(ParameterVec3)))
							{
								light_element.getOrCreateUniform<UniformVec3Instance>(name)->setValue(static_cast<ParameterVec3*>(param)->mValue);
							}
							else
							{
								errorState.fail("Unsupported member data type");
								return false;
							}

						}
						else if (value_member->mType == EShaderVariableValueType::Mat4)
						{
							auto* param = light->getLightUniform(name);
							if (!errorState.check(param != nullptr, "Unsupported member data type"))
								return false;
							light_element.getOrCreateUniform<UniformMat4Instance>(name)->setValue(static_cast<ParameterMat4*>(param)->mValue);
						}
						else
						{
							errorState.fail("Unsupported member data type");
							return false;
						}
					}
					else
					{
						errorState.fail("Unsupported member data type");
						return false;
					}
				}

				// Shadows
				if (light->isShadowEnabled())
				{
					const auto light_view_projection = light->getShadowCamera()->getProjectionMatrix() * light->getShadowCamera()->getViewMatrix();
					light_element.getOrCreateUniform<UniformMat4Instance>(uniform::light::lightViewProjection)->setValue(light_view_projection);

					auto shadow_sampler_array = mesh_comp->getMaterialInstance().getOrCreateSamplerFromResource(*mSamplerResource, errorState);
					if (shadow_sampler_array == nullptr)
						return false;

					auto* instance = static_cast<Sampler2DArrayInstance*>(shadow_sampler_array);
					if (count >= instance->getNumElements())
						continue;

					instance->setTexture(count, *mLightDepthTextureMap[light]);
				}
				++count;
			}
			light_count->setValue(count);
		}
		return true;
	}


	void RenderAdvancedService::preShutdown()
	{
		mShadowTextureDummy.reset();
		mSamplerResource.reset();
	}


	void RenderAdvancedService::preResourcesLoaded()
	{

	}


	void RenderAdvancedService::postResourcesLoaded()
	{
		// We now know the number of light components in the scene and can initialize resources accordingly
		utility::ErrorState error_state;
		if (!initShadowMappingResources(error_state))
		{
			nap::Logger::error(error_state.toString());
			assert(false);
		}
	}


	bool RenderAdvancedService::initShadowMappingResources(utility::ErrorState& errorState)
	{
		// Shadow maps
		mLightDepthTextureMap.clear();
		mLightDepthTextureMap.reserve(mLightComponents.size());

		mLightDepthTargetMap.clear();
		mLightDepthTargetMap.reserve(mLightComponents.size());

		auto* configuration = getConfiguration<RenderAdvancedServiceConfiguration>();
		for (const auto& light : mLightComponents)
		{
			// Texture
			auto shadow_map = std::make_unique<DepthRenderTexture2D>(getCore());
			shadow_map->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
			shadow_map->mWidth = configuration->mShadowMapSize;
			shadow_map->mHeight = configuration->mShadowMapSize;
			shadow_map->mUsage = ETextureUsage::Static;
			shadow_map->mFormat = DepthRenderTexture2D::EDepthFormat::D16;
			shadow_map->mColorSpace = EColorSpace::Linear;
			shadow_map->mClearValue = 1.0f;
			shadow_map->mFill = true;

			if (!shadow_map->init(errorState))
			{
				errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
				return false;
			}

			const auto it_tex = mLightDepthTextureMap.insert({ light, std::move(shadow_map) }).first;

			// Target
			auto shadow_target = std::make_unique<DepthRenderTarget>(getCore());
			shadow_target->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
			shadow_target->mClearValue = 1.0f;
			shadow_target->mDepthTexture = it_tex->second.get();
			shadow_target->mRequestedSamples = ERasterizationSamples::One;
			shadow_target->mSampleShading = false;

			if (!shadow_target->init(errorState))
			{
				errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
				return false;
			}

			mLightDepthTargetMap.insert({ light, std::move(shadow_target) });
		}
		return true;
	}


	void RenderAdvancedService::registerLightComponent(LightComponentInstance& light)
	{
		auto found_it = std::find_if(mLightComponents.begin(), mLightComponents.end(), [input = &light](const auto& it)
		{
			return it == input;
		});
		if (found_it == mLightComponents.end())
			mLightComponents.emplace_back(&light);
	}


	void RenderAdvancedService::removeLightComponent(LightComponentInstance& light)
	{
		{
			auto found_it = std::find_if(mLightDepthTargetMap.begin(), mLightDepthTargetMap.end(), [input = &light](const auto& it)
			{
				return it.first == input;
			});
			assert(found_it != mLightDepthTargetMap.end());
			mLightDepthTargetMap.erase(found_it);
		}
		{
			auto found_it = std::find_if(mLightDepthTextureMap.begin(), mLightDepthTextureMap.end(), [input = &light](const auto& it)
			{
				return it.first == input;
			});
			assert(found_it != mLightDepthTextureMap.end());
			mLightDepthTextureMap.erase(found_it);
		}
		{
			auto found_it = std::find_if(mLightComponents.begin(), mLightComponents.end(), [input = &light](const auto& it)
			{
				return it == input;
			});
			assert(found_it != mLightComponents.end());
			mLightComponents.erase(found_it);
		}
	}
}
