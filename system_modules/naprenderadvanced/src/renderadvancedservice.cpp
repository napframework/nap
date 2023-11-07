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

#include <parametervec.h>
#include <parameternumeric.h>
#include <parametermat.h>
#include <parametercolor.h>

RTTI_BEGIN_CLASS(nap::RenderAdvancedServiceConfiguration)
	RTTI_PROPERTY("ShadowDepthFormat",		&nap::RenderAdvancedServiceConfiguration::mDepthFormat,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ShadowDepthFormatCube",	&nap::RenderAdvancedServiceConfiguration::mDepthFormatCube,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("EnableShadowMapping",	&nap::RenderAdvancedServiceConfiguration::mEnableShadowMapping,	nap::rtti::EPropertyMetaData::Default)
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
		uniform::light::origin,
		uniform::light::direction,
		uniform::light::flags
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

	RenderAdvancedService::RenderAdvancedService(ServiceConfiguration* configuration) :
		Service(configuration) { }


	void RenderAdvancedService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
	{
		dependencies.emplace_back(RTTI_OF(RenderService));
	}


	bool RenderAdvancedService::init(nap::utility::ErrorState& errorState)
	{
		mRenderService = getCore().getService<RenderService>();
		assert(mRenderService != nullptr);

		// Ensure the initialized Vulkan API version meets the render advanced service requirement
		if (mRenderService->getVulkanVersionMajor() <= mRequiredVulkanVersionMajor)
		{
			if (mRenderService->getVulkanVersionMajor() < mRequiredVulkanVersionMajor || mRenderService->getVulkanVersionMinor() < mRequiredVulkanVersionMinor)
			{
				errorState.fail("%s: Vulkan API Version %d.%d required", this->get_type().get_name().to_string().c_str(), mRequiredVulkanVersionMajor, mRequiredVulkanVersionMinor);
				return false;
			}
		}

		// Get configuration
		auto* configuration = getConfiguration<RenderAdvancedServiceConfiguration>();
		if (!errorState.check(configuration != nullptr, "Failed to get nap::RenderAdvancedServiceConfiguration"))
			return false;

        // Enable shadow mapping
        mShadowMappingEnabled = configuration->mEnableShadowMapping;

#ifdef RENDERADVANCED_RPI
        // Disable shadows on Vulkan version <1.1
        if (!errorState.check(mRenderService->getVulkanVersionMajor() >= 1, "Vulkan version 1.0+ required"))
            return false;

        if (mRenderService->getVulkanVersionMajor() == 1 && mRenderService->getVulkanVersionMinor() < 1)
        {
            nap::Logger::warn("Shadow mapping is not supported for RPI with Vulkan version <1.1");
            mShadowMappingEnabled = false;
        }
#endif

        // Log shadow mapping information
        nap::Logger::info("Shadow mapping: %s", mShadowMappingEnabled ? "enabled" : "disabled");
        if (mShadowMappingEnabled)
        {
            auto quad_format_type = RTTI_OF(DepthRenderTexture2D::EDepthFormat).get_enumeration();
            nap::Logger::info("Shadow quad format: %s", quad_format_type.value_to_name(configuration->mDepthFormat).to_string().c_str());

            auto cube_format_type = RTTI_OF(DepthRenderTextureCube::EDepthFormat).get_enumeration();
            nap::Logger::info("Shadow cube format: %s", cube_format_type.value_to_name(configuration->mDepthFormatCube).to_string().c_str());
        }

        // Create and manage a shadow texture dummy for valid shadow samplers
		mShadowTextureDummy = std::make_unique<DepthRenderTexture2D>(getCore());
		mShadowTextureDummy->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(DepthRenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
		mShadowTextureDummy->mWidth = 1;
		mShadowTextureDummy->mHeight = 1;
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
		mSampler2DResource = std::make_unique<Sampler2DArray>(mMaxLightCount);
		mSampler2DResource->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(Sampler2DArray).get_name().to_string().c_str(), math::generateUUID().c_str());
		mSampler2DResource->mName = sampler::light::shadowMaps;

		// Copy pointers
		for (auto& tex : mSampler2DResource->mTextures)
			tex = mShadowTextureDummy.get();

		mSampler2DResource->mBorderColor = EBorderColor::IntOpaqueBlack;
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
		mSamplerCubeResource = std::make_unique<SamplerCubeArray>(mMaxLightCount);
		mSamplerCubeResource->mID = utility::stringFormat("%s_Dummy_%s", RTTI_OF(SamplerCubeArray).get_name().to_string().c_str(), math::generateUUID().c_str());
		mSamplerCubeResource->mName = sampler::light::cubeShadowMaps;

		// Copy pointers
		for (auto& tex : mSamplerCubeResource->mTextures)
			tex = &mRenderService->getEmptyTextureCube();

		mSamplerCubeResource->mBorderColor = EBorderColor::IntOpaqueBlack;
		mSamplerCubeResource->mAddressModeHorizontal = EAddressMode::ClampToEdge;
		mSamplerCubeResource->mAddressModeVertical = EAddressMode::ClampToEdge;
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
		mCubeMapMaterial = mRenderService->getOrCreateMaterial<CubeMapShader>(errorState);
		if (!errorState.check(mCubeMapMaterial != nullptr, "Failed to create CubeMap material"))
			return false;

		mCubeMapMaterial->mBlendMode = EBlendMode::Opaque;
		mCubeMapMaterial->mDepthMode = EDepthMode::NoReadWrite;
		if (!mCubeMapMaterial->init(errorState))
			return false;

		mCubeMaterialInstanceResource = std::make_unique<MaterialInstanceResource>();
		mCubeMaterialInstanceResource->mMaterial = mCubeMapMaterial;
		mCubeMaterialInstance = std::make_unique<MaterialInstance>();
		if (!mCubeMaterialInstance->init(*mRenderService, *mCubeMaterialInstanceResource, errorState))
			return false;

		return true;
	}


	void RenderAdvancedService::renderShadows(const std::vector<RenderableComponentInstance*>& renderComps, bool updateMaterials, RenderMask renderMask)
	{
		auto render_comps = mRenderService->filterObjects(renderComps, renderMask);
		if (updateMaterials)
		{
			utility::ErrorState error_state;
			if (!pushLights(render_comps, error_state))
			{
				nap::Logger::error(error_state.toString());
				assert(false);
			}
		}

        // Bail if shadow mapping is disabled
        if (!isShadowMappingEnabled())
            return;

		// Evaluate registered light components
		for (const auto& light : mLightComponents)
		{
			// Skip rendering the shadow map when the light intensity is zero
			if (!light->isEnabled() || light->getIntensity() <= math::epsilon<float>())
				continue;

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
					mRenderService->renderObjects(*target, *shadow_camera, render_comps);
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
					cube_target->render(*persp_camera, [rs = mRenderService, comps = render_comps](CubeDepthRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)
					{
						// NOTE: This overload of renderObjects does no filtering of non-ortho comps
						rs->renderObjects(target, projection, view, comps, std::bind(&sorter::sortObjectsByDepth, std::placeholders::_1, std::placeholders::_2));
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
		// TODO: Cache light uniforms

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
			// Shader interface for lights
			auto* light_struct = mesh_comp->getMaterialInstance().getOrCreateUniform(uniform::lightStruct);
			auto* light_count = light_struct->getOrCreateUniform<UniformUIntInstance>(uniform::light::count);

			// Set light count to zero and exit early
			if (disableLighting)
			{
				light_count->setValue(0);
				continue;
			}
            light_count->setValue(mLightComponents.size());

			auto* light_array = light_struct->getOrCreateUniform<UniformStructArrayInstance>(uniform::light::lights);

			uint light_index = 0;
			for (const auto& light : mLightComponents)
			{
				if (light_index >= mMaxLightCount)
					break;

				// Fetch flags
				auto it_flags = mLightFlagsMap.find(light);
				assert(it_flags != mLightFlagsMap.end());

				// Set light uniform defaults
				auto& light_element = light_array->getElement(light_index);
				light_element.getOrCreateUniform<UniformUIntInstance>(uniform::light::flags)->setValue(it_flags->second);
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
                    assert(struct_decl != nullptr);

                    auto* member = struct_decl->findMember(name);
					if (!errorState.check(member != nullptr, "Missing uniform with name '%s' in light '%s'", name.c_str(), light->mID.c_str()))
						return false;

					if (member->get_type().is_derived_from(RTTI_OF(ShaderVariableValueDeclaration)))
					{
						auto* value_member = static_cast<const ShaderVariableValueDeclaration*>(member);
                        assert(value_member != nullptr);

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
                ++light_index;
			}
        }

        if (!isShadowMappingEnabled())
            return true;

        // Shadow data
        for (auto& mesh_comp : filtered_mesh_comps)
        {
			// Ensure the shader interface is valid
            auto* shadow_struct = mesh_comp->getMaterialInstance().getOrCreateUniform(uniform::shadowStruct);
            auto* view_matrix_array = shadow_struct->getOrCreateUniform<UniformMat4ArrayInstance>(uniform::shadow::lightViewProjectionMatrix);
            auto* near_far_array = shadow_struct->getOrCreateUniform<UniformVec2ArrayInstance>(uniform::shadow::nearFar);
            auto* strength_array = shadow_struct->getOrCreateUniform<UniformFloatArrayInstance>(uniform::shadow::strength);
            auto* shadow_flags = shadow_struct->getOrCreateUniform<UniformUIntInstance>(uniform::shadow::flags);
            auto* light_count = shadow_struct->getOrCreateUniform<UniformUIntInstance>(uniform::shadow::count);

            // Set light count to zero and exit early
            if (disableLighting)
            {
                light_count->setValue(0);
                continue;
            }
            light_count->setValue(mLightComponents.size());

			// Set shadow flags
			shadow_flags->setValue(getShadowFlags(mLightComponents));

            uint light_index = 0;
            for (const auto& light : mLightComponents)
            {
                if (light_index >= mMaxLightCount)
                    break;

                if (light->isShadowEnabled())
                {
                    // Set light view projection matrix in shadow struct
                    const auto light_view_projection = light->getShadowCamera()->getRenderProjectionMatrix() * light->getShadowCamera()->getViewMatrix();
                    view_matrix_array->setValue(light_view_projection, light_index);

                    // Set near/far plane and strength in shadow struct
                    glm::vec2 near_far = { light->getShadowCamera()->getNearClippingPlane(), light->getShadowCamera()->getFarClippingPlane() };
                    near_far_array->setValue(near_far, light_index);
                    strength_array->setValue(light->getShadowStrength(), light_index);

                    // Fetch flags
                    auto it_flags = mLightFlagsMap.find(light);
                    assert(it_flags != mLightFlagsMap.end());
					updateLightFlags(*light, it_flags->second);

                    switch (light->getShadowMapType())
                    {
                        case EShadowMapType::Quad:
                        {
                            auto shadow_sampler_array = mesh_comp->getMaterialInstance().getOrCreateSamplerFromResource(*mSampler2DResource, errorState);
                            if (shadow_sampler_array != nullptr)
                            {
                                auto *instance = static_cast<Sampler2DArrayInstance *>(shadow_sampler_array);
                                assert(instance != nullptr);

                                if (light_index >= instance->getNumElements())
                                {
                                    assert(false);
                                    continue;
                                }

                                const auto it = mLightDepthMap.find(light);
                                assert(it != mLightDepthMap.end());
                                instance->setTexture(getLightIndex(it_flags->second), *it->second->mTexture);
                            }
                            break;
                        }
                        case EShadowMapType::Cube: {
                            auto shadow_sampler_array = mesh_comp->getMaterialInstance().getOrCreateSamplerFromResource(*mSamplerCubeResource, errorState);
                            if (shadow_sampler_array != nullptr) {
                                auto *instance = static_cast<SamplerCubeArrayInstance *>(shadow_sampler_array);
                                assert(instance != nullptr);

                                if (light_index >= instance->getNumElements()) {
                                    assert(false);
                                    continue;
                                }

                                const auto it = mLightCubeMap.find(light);
                                assert(it != mLightCubeMap.end());
                                instance->setTexture(getLightIndex(it_flags->second), *it->second->mTexture);
                            }
                            break;
                        }
                        default:
                            NAP_ASSERT_MSG(false, "Unsupported shadow map type");
                            return false;
                    }
                }
                ++light_index;
            }
        }
		return true;
	}


	bool RenderAdvancedService::pushLights(const std::vector<RenderableComponentInstance*>& renderComps, utility::ErrorState& errorState)
	{
		return pushLights(renderComps, false, errorState);
	}


	bool RenderAdvancedService::initCubeMapTargets(utility::ErrorState& errorState)
	{
		// Cube maps from file
		auto cube_maps = mRenderService->getCore().getResourceManager()->getObjects<CubeMapFromFile>();
		if (!cube_maps.empty())
		{
			// Prepare render targets
			mCubeMapFromFileTargets.reserve(cube_maps.size());
			for (auto& cm : cube_maps)
			{
				// Cube map from file render target
				auto rt = std::make_unique<CubeRenderTarget>(getCore());
				rt->mID = utility::stringFormat("%s_%s", RTTI_OF(CubeRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
				rt->mClearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
				rt->mSampleShading = cm->mSampleShading;
				rt->mUpdateLODs = true;
				rt->mCubeTexture = cm;

				if (!rt->init(errorState))
				{
					errorState.fail("%s: Failed to initialize cube map from file render target", RTTI_OF(RenderAdvancedService).get_name().to_string().c_str());
					return false;
				}
				mCubeMapFromFileTargets.emplace_back(std::move(rt));
			}
		}
		return true;
	}


	void RenderAdvancedService::onPreRenderCubeMaps(RenderService& renderService)
	{
		// Prerender shadow maps here
		auto cube_maps = mRenderService->getCore().getResourceManager()->getObjects<CubeMapFromFile>();

		uint count = 0U;
		for (auto& cube_map : cube_maps)
		{
			auto projection_matrix = glm::perspective(90.0f, 1.0f, 0.01f, 1000.0f);
			auto origin = glm::vec3(0.0f, 0.0f, 0.0f);

			auto& rt = mCubeMapFromFileTargets[count];
			rt->render(origin, projection_matrix, [rs = &renderService, cm = cube_map.get(), mesh = mNoMesh.get(), mtl = mCubeMaterialInstance.get()]
				(CubeRenderTarget& target, const glm::mat4& projection, const glm::mat4& view)
			{
				auto* ubo = mtl->getOrCreateUniform(uniform::cubemap::uboStruct);
				if (ubo != nullptr)
				{
					ubo->getOrCreateUniform<UniformUIntInstance>(uniform::cubemap::face)->setValue(target.getLayerIndex());
				}

				// Set equirectangular texture to convert
				auto* sampler = mtl->getOrCreateSampler<Sampler2DInstance>(uniform::cubemap::sampler::equiTexture);
				if (sampler != nullptr)
					sampler->setTexture(*cm->mSourceTexture);

				// Get valid descriptor set
				const DescriptorSet& descriptor_set = mtl->update();

				// Get pipeline to to render with
				utility::ErrorState error_state;
				RenderService::Pipeline pipeline = rs->getOrCreatePipeline(target, *mesh, *mtl, error_state);
						
				vkCmdBindPipeline(rs->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
				vkCmdBindDescriptorSets(rs->getCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

				// Bufferless drawing with the cube map shader
				vkCmdDraw(rs->getCurrentCommandBuffer(), 3, 1, 0, 0);
			});
			++count;
		}
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

		mPreRenderCubeMapsCommand.reset();
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
			return;
		}

		// Gather initialized cube maps and perform a headless render pass to write the cube faces
		if (!initCubeMapTargets(error_state))
		{
			nap::Logger::error(error_state.toString());
			assert(false);
			return;
		}

		// Queue cube maps pre-render command if they are present in the scene
		if (!mCubeMapFromFileTargets.empty())
		{
			mPreRenderCubeMapsCommand = std::make_unique<PreRenderCubeMapsCommand>(*this);
			mRenderService->queueRenderCommand(mPreRenderCubeMapsCommand.get());
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
			switch (light->getShadowMapType())
			{
			case EShadowMapType::Quad:
			{
				// Texture
				auto shadow_map = std::make_unique<DepthRenderTexture2D>(getCore());
				shadow_map->mID = utility::stringFormat("%s_ShadowMap_%s", RTTI_OF(DepthRenderTexture2D).get_name().to_string().c_str(), math::generateUUID().c_str());
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
				shadow_target->mID = utility::stringFormat("%s_ShadowMap_%s", RTTI_OF(DepthRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
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
				cube_map->mID = utility::stringFormat("%s_ShadowMap_%s", RTTI_OF(DepthRenderTextureCube).get_name().to_string().c_str(), math::generateUUID().c_str());
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
				cube_target->mID = utility::stringFormat("%s_ShadowMap_%s", RTTI_OF(CubeDepthRenderTarget).get_name().to_string().c_str(), math::generateUUID().c_str());
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

			const uint index = shadow_map_indices[light->getShadowMapType()]++;
			const auto it_flag = mLightFlagsMap.insert({ light, getLightFlags(*light, index) });
			assert(it_flag.second);
		}
		mShadowResourcesCreated = true;
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

        // Warn when a light component is ignored
        if (mLightComponents.size() > mMaxLightCount)
            nap::Logger::warn("'%s' exceeds the maximum of %d nap::LightComponent(s)", light.mID.c_str(), mMaxLightCount);
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
