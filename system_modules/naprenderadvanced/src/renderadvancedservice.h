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

// Local includes
#include "cuberendertarget.h"
#include "cubedepthrendertarget.h"
#include "lightcomponent.h"
#include "lightflags.h"
#include "rendermask.h"
#include "nomesh.h"
#include "rendercommand.h"

namespace nap
{
	// Forward declares
	class RenderService;
	class RenderableComponentInstance;
	class Material;

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
	 * Render Advanced Service
	 */
	class NAPAPI RenderAdvancedService : public Service
	{
		friend class LightComponentInstance;
		RTTI_ENABLE(Service)
	public:
		// Default Constructor
		RenderAdvancedService(ServiceConfiguration* configuration);

		/**
		 * Use this call to register service dependencies
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
		 * Use this function to close service specific handles, drivers or devices
		 * When service B depends on A, Service B is shutdown before A
		 */
		virtual void preShutdown() override;

		/**
		 * Invoked when the resource manager is about to load resources
		 */
		virtual void preResourcesLoaded() override;

		/**
		 * Invoked after the resource manager successfully loaded resources
		 */
		virtual void postResourcesLoaded() override;

        /**
         * @return whether shadow mapping is enabled
         */
        bool isShadowMappingEnabled() const { return mShadowMappingEnabled; }

        /**
         * @return the registered light components
         */
       const std::vector<LightComponentInstance*>& getLights() { return mLightComponents; }

		/**
		 * Render shadows
		 */
		void renderShadows(const std::vector<RenderableComponentInstance*>& renderComps, bool updateMaterials = true, RenderMask renderMask = 0U);

		/**
		 * Push light data
		 */
		bool pushLights(const std::vector<RenderableComponentInstance*>& renderComps, utility::ErrorState& errorState);

	private:
		bool pushLights(const std::vector<RenderableComponentInstance*>& renderComps, bool disableLighting, utility::ErrorState& errorState);
		void registerLightComponent(LightComponentInstance& light);
		void removeLightComponent(LightComponentInstance& light);

		bool initServiceResources(utility::ErrorState& errorState);
		bool initCubeMapTargets(utility::ErrorState& errorState);
		void onPreRenderCubeMaps(RenderService& renderService);

		class PreRenderCubeMapsCommand : public HeadlessCommand
		{
			RTTI_ENABLE(HeadlessCommand)
		public:
			PreRenderCubeMapsCommand(RenderAdvancedService& renderAdvancedService) :
				mRenderAdvancedService(renderAdvancedService), HeadlessCommand() {}

			RenderAdvancedService& mRenderAdvancedService;
		private:
			virtual void record(RenderService& renderService) const override { mRenderAdvancedService.onPreRenderCubeMaps(renderService); }
		};

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
		RenderService* mRenderService = nullptr;

		// Registered light component instances
		std::vector<LightComponentInstance*> mLightComponents;

		// Shadow mapping
		std::unordered_map<LightComponentInstance*, std::unique_ptr<ShadowMapEntry>> mLightDepthMap;
		std::unordered_map<LightComponentInstance*, std::unique_ptr<CubeMapEntry>> mLightCubeMap;
		std::unordered_map<LightComponentInstance*, LightFlags> mLightFlagsMap;

		std::unique_ptr<Sampler2DArray> mSampler2DResource;
		std::unique_ptr<SamplerCubeArray> mSamplerCubeResource;
		std::unique_ptr<DepthRenderTexture2D> mShadowTextureDummy;

		bool mShadowMappingEnabled = true;
		bool mShadowResourcesCreated = false;

		// Cube maps from file
		std::vector<std::unique_ptr<CubeRenderTarget>> mCubeMapFromFileTargets;

		std::unique_ptr<NoMesh>						mNoMesh;
		std::unique_ptr<MaterialInstanceResource>	mCubeMaterialInstanceResource;
		std::unique_ptr<MaterialInstance>			mCubeMaterialInstance;				///< The MaterialInstance as created from the resource. 
		Material*									mCubeMapMaterial = nullptr;

		std::unique_ptr<PreRenderCubeMapsCommand>	mPreRenderCubeMapsCommand;

		static constexpr const uint mRequiredVulkanVersionMajor = 1;
		static constexpr const uint mRequiredVulkanVersionMinor = 0;
		static constexpr const uint mMaxLightCount = 8;
	};
}
