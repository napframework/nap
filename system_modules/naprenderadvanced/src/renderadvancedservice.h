/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <depthrendertarget.h>
#include <rendertexturecube.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <scene.h>

// Local includes
#include "cuberendertarget.h"
#include "cubedepthrendertarget.h"
#include "lightcomponent.h"
#include "lightflags.h"
#include "rendertag.h"
#include "emptymesh.h"
#include "cubemapfromfile.h"

namespace nap
{
	// Forward declares
	class RenderService;
	class RenderableComponentInstance;
	class Material;

	// Light scene identifier
	namespace scene
	{
		namespace light
		{
			inline constexpr const char* id = "lightscene";
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Render Advanced Service
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI RenderAdvancedServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
		friend class PreRenderCubeMapsCommand;
	public:
		virtual rtti::TypeInfo getServiceType() const;

		DepthRenderTexture2D::EDepthFormat mDepthFormat			= DepthRenderTexture2D::EDepthFormat::D16;		///< Quad shadow map depth format
		DepthRenderTextureCube::EDepthFormat mDepthFormatCube	= DepthRenderTextureCube::EDepthFormat::D16;	///< Cube shadow map depth format
	    bool mEnableShadowMapping                               = true;                                         ///< Whether shadow mapping is enabled
    };


	/**
	 * Supplementary interface for a Vulkan Render (2D/3D) and Vulkan Compute operations. This service keeps track of light
	 * components, shadow rendering and cube maps. Resources that are required to prepare these objects (e.g. render
	 * targets, textures, materials etc.) are created after resource initialization and managed at runtime.
	 *
	 * The RenderAdvanced service adds a light system to the render engine which can be used to create great-looking and
	 * consistent lighting setups fast. On initialization, each light component sets up its own light uniform data and
	 * registers itself at the RenderAdvanced service. In order for shaders to be compatible with the light system they
	 * must include an uniform struct with the name `light`, and additionally `shadow` when shadows are supported.
	 * Additional data related to the material surface is excluded from the system and must be set by the user. In the
	 * case of the `nap::BlinnPhongShader` these include `ambient`, `diffuse` and `specular` because they relate to
	 * material properties rather than light properties. The maximum number of lights per scene is always limited to
	 * `getMaximumLightCount`, superfluous lights are ignored. Rendering with lights requires an additional call to
	 * `pushLights` with the render components whose material instances you wish to update light uniforms of.
	 * Alternatively, `renderShadows` with the `updateMaterials` argument set to `true` updates light uniforms and renders
	 * a shadow map. Light components are deregistered and registered again when appropriate on hot-reloads.
	 *
	 * `nap::CubeMapFromFile` objects are gathered after initialization to setup up a render command that generates
	 * cube maps from the supplemented equirectangular image files.
	 *
	 * ~~~~~{.cpp}
	 *  // Handles render commands in the headless render command queue,
	 *  // including operations to pre-render `nap::CubeMapFromFile`
	 *	if (mRenderService->beginHeadlessRecording())
	 *	{
	 *		mRenderAdvancedService->renderShadows(render_comps, true, shadow_mask);
	 *		mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
	 * 
	 * The depth format of quad and cube shadow maps can be changed in the `nap::RenderAdvancedServiceConfiguration` as
	 * well as whether shadow mapping is enabled at all.
	 *
	 * The RenderAdvanced service also adds a number of useful shader include files `.glslinc` which can be found in the
	 * module's data directory. To include a shader file simply add the following `#extension` line to your shader file.
	 * 
	 * ~~~~~{.vert}{.frag}
	 *	#extension GL_GOOGLE_include_directive : enable
	 *	#include "utils.glslinc"
	 * ~~~~~
	 */
	class NAPAPI RenderAdvancedService : public Service
	{
		friend class LightComponentInstance;
		friend class CubeMapFromFile;
		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		RenderAdvancedService(ServiceConfiguration* configuration);

        /**
		 * Returns if global shadow mapping is enabled 
         * @return whether global shadow mapping is enabled.
         */
        bool isShadowMappingEnabled() const													{ return mShadowMappingEnabled; }

        /**
		 * Returns total number of registered lights 
         * @return total number of registered light components.
         */
       const std::vector<LightComponentInstance*>& getLights()								{ return mLightComponents; }

		/**
		 * Renders shadow maps using the specified render components for all registered lights. Must be recorded to a headless
		 * command buffer. The shadow maps that are updated are managed by the RenderAdvanced service. This call also updates
		 * the appropriate uniforms and samplers in the material instances of the render components passed into `renderComps`.
		 *
		 * ~~~~~{.cpp}
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		mRenderAdvancedService->renderShadows(render_comps, true, shadow_mask);
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 * 
		 * @param renderComps the render components to include in the shadow map, and whose uniforms and samplers must be updated
		 * @param updateMaterials whether to update uniform and sampler data with a call to `pushLights`
		 * @param renderMask render mask specifying what components to include in the shadow map 
		 */
		void renderShadows(const std::vector<RenderableComponentInstance*>& renderComps, bool updateMaterials = true, RenderMask renderMask = 0);

		/**
		 * Renders the origin gnomon including optional frustrum of all enabled lights to the requested render target.
		 * The objects to render are sorted using the default sort function (front-to-back).
		 * The sort function is provided by the render service itself, using the default NAP DepthSorter.
		 */
		void renderLocators(IRenderTarget& renderTarget, CameraComponentInstance& camera, bool drawFrustrum);

		/**
		 * Push light data to the shader programs of the given render components.
		 * This function must be called exactly once before rendering when shadows are **not** used, ie: 'renderShadows' is not invoked.
		 *
		 * This call searches for a method called 'getOrCreateMaterial' to locate and extract the program from the component.
		 * If the shader program cannot be found, no action is taken, as the method is not exposed via RTTI.
		 * Refer to 'RenderableMeshComponentInstance::getOrCreateMaterial' for an example.
		 * 
		 * Additional data related to the material surface is excluded from the system and must be set by the user.
		 * 
		 * In order for shaders to be compatible with the light system they must include an uniform struct with 
		 * the name 'light', and additionally 'shadow' when shadows are supported.
		 *
		 * ~~~~~{.cpp}
		 *	if (mRenderService->beginHeadlessRecording())
		 *	{
		 *		mRenderAdvancedService->pushLights(render_comps, error_state);
		 *		mRenderService->endHeadlessRecording();
		 *	}
		 * ~~~~~
		 * @param renderComps the render components whose uniforms and samplers must be updated
		 */
		void pushLights(const std::vector<RenderableComponentInstance*>& renderComps);

		/**
		 * @return the maximum number of lights supported by the RenderAdvanced light system.
		 */
		static uint getMaximumLightCount()													{ return mMaxLightCount; }

	protected:
		/**
		 * Registers service dependencies
		 * A service that depends on another service is initialized after all it's associated dependencies
		 * This will ensure correct order of initialization, update calls and shutdown of all services
		 * @param dependencies rtti information of the services this service depends on
		 */
		virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;

		/**
		 * Initializes the service
		 * @param errorState contains the error message on failure
		 * @return if the video service was initialized correctly
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

		/**
		 * Invoked when exiting the main loop, after app shutdown is called
		 * Use this function if your service needs to reset its state before resources are destroyed
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void preShutdown() override;

		/**
		 * Invoked when exiting the main loop, after app shutdown is called
		 * Use this function to close service specific handles, drivers or devices
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void shutdown() override;

		/**
		 * Invoked when the resource manager is about to load resources. This is when essential RenderAdvanced resources are
		 * created as resources can now be accessed in the scene and light components have been registered.
		 */
		virtual void preResourcesLoaded() override;

		/**
		 * Invoked after the resource manager successfully loaded resources.
		 */
		virtual void postResourcesLoaded() override;

		/**
		 * Spawns an entity camera hierarchy at runtime.
		 * This entity is spawned into a dedicated light scene, independent from the regular user scene.
		 * @param entity the entity resource to spawning fails
		 * @return the spawned light entity instance, nullptr when invalid
		 * @param error contains the error if spawning fails.
		 */
		SpawnedEntityInstance spawn(const nap::Entity& entity, nap::utility::ErrorState& error);

		/**
		 * Destroys an entity hierarchy at runtime.
		 * The entity to destroy must have been created using the spawn method above
		 * @param entityInstance the spawned entity instance
		 */
		void destroy(SpawnedEntityInstance& entityInstance);

		/**
		 * Invoked by core in the app loop. Update order depends on service dependency
		 * This call is invoked after the application update call
		 * @param deltaTime: the time in seconds between calls
		 */
		virtual void postUpdate(double deltaTime);

	private:
		void pushLightsInternal(const std::vector<MaterialInstance*>& materials);
		void registerLightComponent(LightComponentInstance& light);
		void removeLightComponent(LightComponentInstance& light);
		void registerCubeMap(CubeMapFromFile& cubemap);
		void removeCubeMap(CubeMapFromFile& cubemap);
		bool initServiceResources(utility::ErrorState& errorState);

		/**
		 * A depth render target and texture pair used for shadow rendering.
		 */
		struct ShadowMapEntry
		{
			ShadowMapEntry(std::unique_ptr<DepthRenderTarget> target, std::unique_ptr<DepthRenderTexture2D> texture) :
				mTarget(std::move(target)), mTexture(std::move(texture))
			{
				mTarget->mDepthTexture = mTexture.get();
			}

			std::unique_ptr<DepthRenderTarget> mTarget;
			std::unique_ptr<DepthRenderTexture2D> mTexture;
		};

		/**
		 * A cube depth render target and texture pair used for shadow rendering.
		 */
		struct CubeMapEntry
		{
			CubeMapEntry(std::unique_ptr<CubeDepthRenderTarget> target, std::unique_ptr<DepthRenderTextureCube> texture) :
				mTarget(std::move(target)), mTexture(std::move(texture))
			{
				mTarget->mCubeDepthTexture = mTexture.get();
			}

			std::unique_ptr<CubeDepthRenderTarget> mTarget;
			std::unique_ptr<DepthRenderTextureCube> mTexture;
		};

		// Reference to render service
		RenderService* mRenderService = nullptr;										///< Reference to regular render service

		// Registered light component instances
		std::vector<LightComponentInstance*> mLightComponents;							///< List of all registered light components

		// Shadow mapping
		std::unordered_map<LightComponentInstance*, std::unique_ptr<ShadowMapEntry>> mLightDepthMap;	///< Links light components to 2d shadow resources
		std::unordered_map<LightComponentInstance*, std::unique_ptr<CubeMapEntry>> mLightCubeMap;		///< Links light components to cube shadow resources
		std::unordered_map<LightComponentInstance*, LightFlags> mLightFlagsMap;							///< Links light components to information bitflags

		std::unique_ptr<Sampler2DArray> mSampler2DResource;								///< Base sampler 2D resource with a reference to a depth texture dummy
		std::unique_ptr<SamplerCubeArray> mSamplerCubeResource;							///< Base cube sampler resource with a reference to a cube depth texture dummy
		std::unique_ptr<DepthRenderTexture2D> mShadowTextureDummy;						///< Shadow depth texture dummy
		std::unique_ptr<nap::Scene> mLightScene = nullptr;								///< Light scene

		bool mShadowMappingEnabled = true;												///< Whether shadow mapping is disabled
		bool mShadowResourcesCreated = false;											///< Whether shadow resources are created, enabled after `initServiceResources` completed successfully

		// Cube maps from file
		std::unordered_map<CubeMapFromFile*, std::unique_ptr<CubeRenderTarget>> mCubeMapTargets;		///< List of all registered cube map from file resources

		std::unique_ptr<EmptyMesh>					mNoMesh;							///< No mesh is required for generating a cube map from an equirectangular texture
		std::unique_ptr<MaterialInstanceResource>	mCubeMaterialInstanceResource;		///< Run-time cube map material instance resource
		std::unique_ptr<MaterialInstance>			mCubeMaterialInstance;				///< The MaterialInstance as created from the resource. 
		Material*									mCubeMapMaterial = nullptr;			///< Run-time cube map material

		static constexpr const uint mRequiredVulkanVersionMajor = 1;					///< Required Vulkan major version to support the render advanced service
		static constexpr const uint mRequiredVulkanVersionMinor = 0;					///< Required Vulkan minor version to support the render advanced service
		static constexpr const uint mMaxLightCount = 8;									///< The maximum number of lights the supported by the render advanced  service light system
	};
}
