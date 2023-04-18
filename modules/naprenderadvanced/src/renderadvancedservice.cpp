/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderadvancedservice.h"
#include "cubemapfromfile.h"
#include "cubemapshader.h"

// External Includes
#include <renderservice.h>
#include <rendercomponent.h>
#include <renderablemeshcomponent.h>
#include <perspcameracomponent.h>
#include <depthsorter.h>
#include <nap/core.h>
#include <rtti/factory.h>
#include <vulkan/vulkan_core.h>
#include <material.h>
#include <renderglobals.h>

#include <parameter.h>
#include <parametervec.h>
#include <parameternumeric.h>
#include <parametermat.h>
#include <parametercolor.h>

RTTI_BEGIN_CLASS(nap::RenderAdvancedServiceConfiguration)
	RTTI_PROPERTY("ShadowDepthFormat",			&nap::RenderAdvancedServiceConfiguration::mDepthFormat,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowDepthFormatCube",		&nap::RenderAdvancedServiceConfiguration::mDepthFormatCube,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderAdvancedService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS
	
namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static
	//////////////////////////////////////////////////////////////////////////

	static const std::vector<const char*> sDefaultUniforms =
	{
		uniform::light::flags,
		uniform::light::origin,
		uniform::light::direction,
		uniform::light::viewProjectionMatrix
	};


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


	static bool isShadowEnabled(uint lightFlags)
	{
		return (lightFlags & (1 << 8U)) > 0;
	}

	static uint getLightType(uint lightFlags)
	{
		return static_cast<uint>((lightFlags >> 16U) & 0xff);
	}

	static uint getLightIndex(uint lightFlags)
	{
		return static_cast<uint>((lightFlags >> 24U) & 0xff);
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
		auto* render_service = getCore().getService<RenderService>();
		assert(render_service != nullptr);

		// Ensure the initialized Vulkan API version meets the render advanced service requirement
		if (render_service->getVulkanVersionMajor() <= mRequiredVulkanVersionMajor)
		{
			if (render_service->getVulkanVersionMajor() < mRequiredVulkanVersionMajor || render_service->getVulkanVersionMinor() < mRequiredVulkanVersionMinor)
			{
				errorState.fail("%s: Vulkan API Version %d.%d required", this->get_type().get_name().to_string().c_str(), mRequiredVulkanVersionMajor, mRequiredVulkanVersionMinor);
				return false;
			}
		}

		// Get configuration
		auto* configuration = getConfiguration<RenderAdvancedServiceConfiguration>();

		// Create and manage a shadow texture dummy for valid shadow samplers
		mShadowTextureDummy = std::make_unique<DepthRenderTexture2D>(getCore());
		mShadowTextureDummy->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(DepthRenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
		mShadowTextureDummy->mWidth = 16;
		mShadowTextureDummy->mHeight = 16;
		mShadowTextureDummy->mUsage = Texture::EUsage::Static;
		mShadowTextureDummy->mDepthFormat = configuration->mDepthFormat;
		mShadowTextureDummy->mColorSpace = EColorSpace::Linear;
		mShadowTextureDummy->mClearValue = 1.0f;
		mShadowTextureDummy->mFill = true;
		if (!mShadowTextureDummy->init(errorState))
		{
			errorState.fail("%s: Failed to create shadow texture dummy", this->get_type().get_name().to_string().c_str());
			return false;
		}

		// Sampler2D
		mSampler2DResource = std::make_unique<Sampler2DArray>(mMaxShadowMapCount);
		mSampler2DResource->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(Sampler2DArray).get_name().to_string().c_str(), math::generateUUID().c_str());
		mSampler2DResource->mName = sampler::light::shadowMaps;

		// Copy pointers
		for (auto& tex : mSampler2DResource->mTextures)
			tex = mShadowTextureDummy.get();

		mSampler2DResource->mBorderColor = EBorderColor::IntOpaqueWhite;
		mSampler2DResource->mAddressModeHorizontal = EAddressMode::ClampToEdge;
		mSampler2DResource->mAddressModeVertical = EAddressMode::ClampToEdge;
		mSampler2DResource->mCompareMode = EDepthCompareMode::LessOrEqual;
		mSampler2DResource->mEnableCompare = true;

		if (!mSampler2DResource->init(errorState))
		{
			errorState.fail("%s: Failed to initialize shadow sampler 2d resource", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
			return false;
		}

		// SamplerCube
		mSamplerCubeResource = std::make_unique<SamplerCubeArray>(mMaxShadowMapCount);
		mSamplerCubeResource->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(SamplerCubeArray).get_name().to_string().c_str(), math::generateUUID().c_str());
		mSamplerCubeResource->mName = sampler::light::cubeShadowMaps;

		// Copy pointers
		for (auto& tex : mSamplerCubeResource->mTextures)
			tex = &render_service->getEmptyTextureCube();

		mSamplerCubeResource->mBorderColor = EBorderColor::IntOpaqueBlack;
		mSamplerCubeResource->mCompareMode = EDepthCompareMode::LessOrEqual;
		mSamplerCubeResource->mEnableCompare = true;
		if (!mSamplerCubeResource->init(errorState))
		{
			errorState.fail("%s: Failed to initialize cube sampler resource", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
			return false;
		}

		// Create nap::NoMesh
		mNoMesh = std::make_unique<NoMesh>(getCore());
		mNoMesh->mID = utility::stringFormat("%s_NoMesh_%s", RTTI_OF(NoMesh).get_name().to_string().c_str(), math::generateUUID().c_str());
		if (!mNoMesh->init(errorState))
		{
			errorState.fail("%s: Failed to initialize cube sampler resource", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
			return false;
		}

		// Create material instance
		mCubeMaterialInstanceResource = std::make_unique<MaterialInstanceResource>();
		mCubeMaterialInstanceResource->mBlendMode = EBlendMode::Opaque;
		mCubeMaterialInstanceResource->mDepthMode = EDepthMode::NoReadWrite;

		mCubeMapMaterial = render_service->getOrCreateMaterial<CubeMapShader>(errorState);
		if (mCubeMapMaterial == nullptr)
			return false;

		if (!mCubeMapMaterial->init(errorState))
			return false;

		mCubeMaterialInstanceResource->mMaterial = mCubeMapMaterial;
		mCubeMaterialInstance = std::make_unique<MaterialInstance>();
		if (!mCubeMaterialInstance->init(*render_service, *mCubeMaterialInstanceResource, errorState))
			return false;

		return true;
	}


	void RenderAdvancedService::update(double deltaTime)
	{
		//
	}


	void RenderAdvancedService::renderShadows(const std::vector<RenderableComponentInstance*>& renderComps, bool updateMaterials, RenderMask renderMask)
	{
		auto* render_service = getCore().getService<RenderService>();
		assert(render_service != nullptr);

		if (updateMaterials)
		{
			utility::ErrorState error_state;
			if (!pushLights(renderComps, error_state))
			{
				nap::Logger::error(error_state.toString());
				assert(false);
			}
		}

		// Evaluate registered light components
		for (const auto& light : mLightComponents)
		{
			// TODO: Only re-render shadow map when light transform or camera matrix has changed
			auto* shadow_camera = light->isShadowEnabled() ? light->getShadowCamera() : nullptr;
			if (shadow_camera != nullptr)
			{
				switch (light->getShadowMapType())
				{
				case EShadowMapType::Quad:
				{
					// One shadow render pass per light
					auto it = mLightDepthMap.find(light);
					assert(it != mLightDepthMap.end());

					auto& target = it->second->mTarget;
					assert(target != nullptr);

					target->beginRendering();
					render_service->renderObjects(*target, *shadow_camera, renderComps, renderMask);
					target->endRendering();
					break;
				}
				case EShadowMapType::Cube:
				{
					// One shadow render pass per light
					auto it = mLightCubeMap.find(light);
					assert(it != mLightCubeMap.end());

					auto& target = it->second->mTarget;
					assert(target != nullptr);

					if (target.get()->get_type() != RTTI_OF(CubeDepthRenderTarget))
					{
						NAP_ASSERT_MSG(false, "Point lights must be linked to nap::CubeDepthRenderTarget");
						continue;
					}

					if (shadow_camera->get_type() != RTTI_OF(PerspCameraComponentInstance))
					{
						NAP_ASSERT_MSG(false, "Lights of type 'Point' must return a nap::PerspCameraComponentInstance");
						continue;
					}

					auto* cube_target = static_cast<CubeDepthRenderTarget*>(target.get());
					auto* persp_camera = static_cast<PerspCameraComponentInstance*>(shadow_camera);
					cube_target->render(*persp_camera, [rs = render_service, comps = renderComps, mask = renderMask](CubeDepthRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)
					{
						// NOTE: This overload of renderObjects does no filtering of non-ortho comps
						rs->renderObjects(target, projection, view, comps, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2), mask);
					});
					break;
				}
				default:
					NAP_ASSERT_MSG(false, "Unsupported shadow map type");
				}
			}
		}
	}


	bool RenderAdvancedService::pushLights(const std::vector<RenderableComponentInstance*>& renderComps, bool disableLighting, utility::ErrorState& errorState)
	{
		// Exit early if there are no lights in the scene
		if (mLightComponents.empty())
			return true;

		// Filter render components
		std::vector<RenderableMeshComponentInstance*> filtered_mesh_comps;
		for (auto& comp : renderComps)
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

			auto* light_count = light_struct->getOrCreateUniform<UniformUIntInstance>(uniform::light::count);
			if (light_count == nullptr)
				continue;

			// Set light count to zero and exit early
			if (disableLighting)
			{
				light_count->setValue(0);
				return true;
			}

			auto* light_array = light_struct->getOrCreateUniform<UniformStructArrayInstance>(uniform::light::lights);
			if (light_array == nullptr)
				continue;

			uint count = 0;
			for (const auto& light : mLightComponents)
			{
				if (count >= light_array->getMaxNumElements())
					break;

				// Fetch element
				auto& light_element = light_array->getElement(count);
				auto it_flags = mLightFlagsMap.find(light);
				assert(it_flags != mLightFlagsMap.end());

				// Light uniform defaults
				const uint light_flags = it_flags->second;
				light_element.getOrCreateUniform<UniformUIntInstance>(uniform::light::flags)->setValue(light_flags);
				light_element.getOrCreateUniform<UniformVec3Instance>(uniform::light::origin)->setValue(light->getLightPosition());
				light_element.getOrCreateUniform<UniformVec3Instance>(uniform::light::direction)->setValue(light->getLightDirection());

				// Light uniform custom
				for (const auto& entry : light->mUniformDataMap)
				{
					const auto name = entry.first;

					// Filter default uniforms
					bool skip = false;
					for (const auto& default_uniform : sDefaultUniforms)
					{
						if (name == default_uniform)
						{
							skip = true;
							break;
						}
					}
					if (skip)
						break;

					auto* struct_decl = static_cast<const ShaderVariableStructDeclaration*>(&light_element.getDeclaration());
					auto* member = struct_decl->findMember(name);
					if (!errorState.check(member != nullptr, "Missing uniform with name '%s' in light '%s'", name.c_str(), light->mID.c_str()))
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

				if (light->isShadowEnabled())
				{
					// Shadows
					auto* render_service = getCore().getService<RenderService>();
					assert(render_service != nullptr);

					const auto light_view_projection = light->getShadowCamera()->getProjectionMatrix() * light->getShadowCamera()->getViewMatrix();
					light_element.getOrCreateUniform<UniformMat4Instance>(uniform::light::viewProjectionMatrix)->setValue(light_view_projection);
					light_element.getOrCreateUniform<UniformVec2Instance>(uniform::light::nearFar)->setValue({ light->getShadowCamera()->getNearClippingPlane(), light->getShadowCamera()->getFarClippingPlane() });

					switch (light->getShadowMapType())
					{
					case EShadowMapType::Quad:
					{
						auto shadow_sampler_array = mesh_comp->getMaterialInstance().getOrCreateSamplerFromResource(*mSampler2DResource, errorState);
						if (shadow_sampler_array != nullptr)
						{
							auto* instance = static_cast<Sampler2DArrayInstance*>(shadow_sampler_array);
							if (count >= instance->getNumElements())
								continue;

							if (light->isShadowEnabled())
							{
								const auto it = mLightDepthMap.find(light);
								assert(it != mLightDepthMap.end());
								instance->setTexture(getLightIndex(light_flags), *it->second->mTexture);
							}
							else
							{
								instance->setTexture(count, *mShadowTextureDummy);
							}
						}
						break;
					}
					case EShadowMapType::Cube:
					{
						auto shadow_sampler_array = mesh_comp->getMaterialInstance().getOrCreateSamplerFromResource(*mSamplerCubeResource, errorState);
						if (shadow_sampler_array != nullptr)
						{
							auto* instance = static_cast<SamplerCubeArrayInstance*>(shadow_sampler_array);
							if (count >= instance->getNumElements())
								continue;

							if (light->isShadowEnabled())
							{
								const auto it = mLightCubeMap.find(light);
								assert(it != mLightCubeMap.end());
								instance->setTexture(getLightIndex(light_flags), *it->second->mTexture);
							}
							else
							{
								instance->setTexture(count, render_service->getEmptyTextureCube());
							}
						}
						break;
					}
					default:
						NAP_ASSERT_MSG(false, "Unsupported shadow map type");
						return false;
					}
				}
				++count;
			}
			light_count->setValue(count);
		}
		return true;
	}


	bool RenderAdvancedService::pushLights(const std::vector<RenderableComponentInstance*>& renderComps, utility::ErrorState& errorState)
	{
		return pushLights(renderComps, false, errorState);
	}


	void RenderAdvancedService::preShutdown()
	{
		mSampler2DResource.reset();
		mSamplerCubeResource.reset();
		mShadowTextureDummy.reset();

		mNoMesh.reset();
		mCubeMaterialInstanceResource.reset();
		mCubeMaterialInstance.reset();
		mCubeMapFromFileTargets.clear();
	}


	void RenderAdvancedService::preResourcesLoaded()
	{
		mShadowResourcesCreated = false;
	}


	void RenderAdvancedService::postResourcesLoaded()
	{
		// We now know the number of light components in the scene and can initialize resources accordingly
		utility::ErrorState error_state;
		if (!initServiceResources(error_state))
		{
			nap::Logger::error(error_state.toString());
			assert(false);
		}
	}


	bool RenderAdvancedService::initServiceResources(utility::ErrorState& errorState)
	{
		if (mShadowResourcesCreated)
			return true;

		// Shadow maps
		mLightDepthMap.clear();
		mLightDepthMap.reserve(mLightComponents.size());

		mLightCubeMap.clear();
		mLightCubeMap.reserve(mLightComponents.size());

		mLightFlagsMap.clear();
		mLightFlagsMap.reserve(mLightComponents.size());

		mCubeMapFromFileTargets.clear();

		auto* configuration = getConfiguration<RenderAdvancedServiceConfiguration>();

		std::map<EShadowMapType, uint> shadow_map_indices;
		for (const auto& variant : RTTI_OF(EShadowMapType).get_enumeration().get_values())
			shadow_map_indices.insert({ variant.get_value<EShadowMapType>(), 0 });

		for (const auto& light : mLightComponents)
		{
			// Flags [index : 8bit][map_id : 8bit][padding : 7bit][shadow : 1bit][type : 8bit]
			uint flags = static_cast<uint>(light->getLightType());
			flags |= static_cast<uint>(light->isShadowEnabled()) << 8U;
			flags |= static_cast<uint>(light->getShadowMapType()) << 16U;

			switch (light->getShadowMapType())
			{
			case EShadowMapType::Quad:
			{
				// Texture
				auto shadow_map = std::make_unique<DepthRenderTexture2D>(getCore());
				shadow_map->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
				shadow_map->mWidth = light->getShadowMapSize();
				shadow_map->mHeight = light->getShadowMapSize();
				shadow_map->mDepthFormat = configuration->mDepthFormat;
				shadow_map->mUsage = Texture::EUsage::Static;
				shadow_map->mColorSpace = EColorSpace::Linear;
				shadow_map->mClearValue = 1.0f;
				shadow_map->mFill = true;

				if (!shadow_map->init(errorState))
				{
					errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
					return false;
				}

				// Target
				auto shadow_target = std::make_unique<DepthRenderTarget>(getCore());
				shadow_target->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
				shadow_target->mDepthTexture = shadow_map.get();
				shadow_target->mClearValue = 1.0f;
				shadow_target->mRequestedSamples = ERasterizationSamples::One;
				shadow_target->mSampleShading = false;

				auto it = mLightDepthMap.insert({ light, std::make_unique<ShadowMapEntry>(std::move(shadow_target), std::move(shadow_map)) });
				assert(it.second);

				if (!it.first->second->mTarget->init(errorState))
				{
					errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
					return false;
				}

				break;
			}
			case EShadowMapType::Cube:
			{
				// Cube Texture
				auto cube_map = std::make_unique<DepthRenderTextureCube>(getCore());
				cube_map->mID = utility::stringFormat("%s_%s", RTTI_OF(DepthRenderTextureCube).get_name().to_string().c_str(), math::generateUUID().c_str());
				cube_map->mWidth = light->getShadowMapSize();
				cube_map->mHeight = light->getShadowMapSize();
				cube_map->mDepthFormat = configuration->mDepthFormatCube;
				cube_map->mColorSpace = EColorSpace::Linear;
				cube_map->mClearValue = 1.0f;
				cube_map->mFill = true;

 				if (!cube_map->init(errorState))
				{
					errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
					return false;
				}

				// Target
				auto cube_target = std::make_unique<CubeDepthRenderTarget>(getCore());
				cube_target->mID = utility::stringFormat("%s_%s", RTTI_OF(CubeDepthRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
				cube_target->mClearValue = 1.0f;
				cube_target->mRequestedSamples = ERasterizationSamples::One;
				cube_target->mSampleShading = false;

				auto it = mLightCubeMap.insert({ light, std::make_unique<CubeMapEntry>(std::move(cube_target), std::move(cube_map)) });
				assert(it.second);

				if (!it.first->second->mTarget->init(errorState))
				{
					errorState.fail("%s: Failed to initialize shadow mapping resources", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
					return false;
				}

				break;
			}
			default:
				NAP_ASSERT_MSG(false, "Unsupported shadow map type");
				return false;
			}

			uint index = shadow_map_indices[light->getShadowMapType()]++;
			flags |= index << 24U;

			const auto it_flag = mLightFlagsMap.insert({ light, flags });
			assert(it_flag.second);
		}
		mShadowResourcesCreated = true;

		// RENDER PREPASS
		auto* render_service = getCore().getService<RenderService>();
		assert(render_service != nullptr);

		// Cube maps from file
		auto cube_maps = render_service->getCore().getResourceManager()->getObjects<CubeMapFromFile>();
		if (!cube_maps.empty())
		{
			mCubeMapFromFileTargets.reserve(cube_maps.size());
			for (auto& cm : cube_maps)
			{
				// Cube map from file render target
				auto rt = std::make_unique<CubeRenderTarget>(getCore());
				rt->mID = utility::stringFormat("%s_%s", RTTI_OF(CubeRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
				rt->mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
				rt->mSampleShading = true;
				rt->mCubeTexture = cm;

				if (!rt->init(errorState))
				{
					errorState.fail("%s: Failed to initialize cube map from file render target", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
					return false;
				}
				mCubeMapFromFileTargets.emplace_back(std::move(rt));
			}

			// Updates GPU data
			render_service->beginFrame();

			if (render_service->beginHeadlessRecording())
			{
				// Prerender shadow maps here
				uint count = 0U;
				for (auto& cm : cube_maps)
				{
					auto commandBuffer = render_service->getCurrentCommandBuffer();
					auto projection_matrix = glm::perspective(90.0f, 1.0f, 0.01f, 1000.0f);
					auto origin = glm::vec3(0.0f, 0.0f, 0.0f);

					auto& rt = mCubeMapFromFileTargets[count];
					rt->render(origin, projection_matrix, [&](CubeRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)
					{
						auto* ubo = mCubeMaterialInstance->getOrCreateUniform("UBO");
						if (ubo != nullptr)
						{
							ubo->getOrCreateUniform<UniformUIntInstance>("face")->setValue(target.getLayerIndex());
						}

						// Set equirectangular texture to convert
						auto* sampler = mCubeMaterialInstance->getOrCreateSampler<Sampler2DInstance>("equiTexture");
						if (sampler != nullptr)
							sampler->setTexture(*cm->mSourceTexture);

						// Get valid descriptor set
						const DescriptorSet& descriptor_set = mCubeMaterialInstance->update();

						// Get pipeline to to render with
						utility::ErrorState error_state;
						RenderService::Pipeline pipeline = render_service->getOrCreatePipeline(*rt, *mNoMesh, *mCubeMaterialInstance, error_state);
						vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
						vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

						// Bufferless drawing with the cube map shader
						vkCmdDraw(commandBuffer, 3, 1, 0, 0);
					});
					++count;
				}
				render_service->endHeadlessRecording();
			}
			render_service->endFrame();
		}

		return true;
	}


	void RenderAdvancedService::registerLightComponent(LightComponentInstance& light)
	{
		auto found_it = std::find_if(mLightComponents.begin(), mLightComponents.end(), [input = &light](const auto& it)
		{
			return it == input;
		});

		if (found_it != mLightComponents.end())
			NAP_ASSERT_MSG(false, "Light component was already registered");

		mLightComponents.emplace_back(&light);
	}

	 
	void RenderAdvancedService::removeLightComponent(LightComponentInstance& light)
	{
		if (!light.isRegistered())
			return;

		if (mShadowResourcesCreated)
		{
			switch (light.getShadowMapType())
			{
			case EShadowMapType::Quad:
			{
				// Shadow resources
				auto found_it = std::find_if(mLightDepthMap.begin(), mLightDepthMap.end(), [input = &light](const auto& it)
					{
						return it.first == input;
					});
				assert(found_it != mLightDepthMap.end());
				mLightDepthMap.erase(found_it);
				break;
			}
			case EShadowMapType::Cube:
			{
				auto found_it = std::find_if(mLightCubeMap.begin(), mLightCubeMap.end(), [input = &light](const auto& it)
					{
						return it.first == input;
					});
				assert(found_it != mLightCubeMap.end());
				mLightCubeMap.erase(found_it);
				break;
			}
			default:
				NAP_ASSERT_MSG(false, "Unsupported shadow map type");
			}

			// Flags
			auto found_it = std::find_if(mLightFlagsMap.begin(), mLightFlagsMap.end(), [input = &light](const auto& it)
				{
					return it.first == input;
				});
			assert(found_it != mLightFlagsMap.end());
			mLightFlagsMap.erase(found_it);
		}

		// Light components
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
