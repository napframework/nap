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
#include "rendermask.h"
#include "nomesh.h"

namespace nap
{
	// Forward declares
	class RenderableComponentInstance;
	class Material;

	//////////////////////////////////////////////////////////////////////////
	// Render Advanced Service
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI RenderAdvancedServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:
		virtual rtti::TypeInfo getServiceType() const;

		uint mShadowMapSize									= 1024U;										///< Shadow Map Size
		uint mShadowCubeMapSize								= 512U;											///< Shadow Cube Map Size
		DepthRenderTexture2D::EDepthFormat mPrecision		= DepthRenderTexture2D::EDepthFormat::D32;		///< Precision
		DepthRenderTextureCube::EDepthFormat mCubePrecision	= DepthRenderTextureCube::EDepthFormat::D32;	///< Cubemap Precision
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
		RenderAdvancedService(ServiceConfiguration* configuration) :
			Service(configuration) { }

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
		 * Invoked by core in the app loop. Update order depends on service dependency
		 * This call is invoked after the resource manager has loaded any file changes but before
		 * the app update call. If service B depends on A, A:s:update() is called before B::update()
		 * @param deltaTime: the time in seconds between calls
		*/
		virtual void update(double deltaTime) override;

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

		// Registered light component instances
		std::vector<LightComponentInstance*> mLightComponents;

		struct ShadowMapEntry
		{
			ShadowMapEntry::ShadowMapEntry(std::unique_ptr<DepthRenderTarget> target, std::unique_ptr<DepthRenderTexture2D> texture) :
				mTarget(std::move(target)), mTexture(std::move(texture))
			{
				mTarget->mDepthTexture = mTexture.get();
			}

			std::unique_ptr<DepthRenderTarget> mTarget;
			std::unique_ptr<DepthRenderTexture2D> mTexture;
		};

		struct CubeMapEntry
		{
			CubeMapEntry::CubeMapEntry(std::unique_ptr<CubeDepthRenderTarget> target, std::unique_ptr<DepthRenderTextureCube> texture) :
				mTarget(std::move(target)), mTexture(std::move(texture))
			{
				mTarget->mCubeDepthTexture = mTexture.get();
			}

			std::unique_ptr<CubeDepthRenderTarget> mTarget;
			std::unique_ptr<DepthRenderTextureCube> mTexture;
		};

		// Shadow mapping
		std::unordered_map<LightComponentInstance*, std::unique_ptr<ShadowMapEntry>> mLightDepthMap;
		std::unordered_map<LightComponentInstance*, std::unique_ptr<CubeMapEntry>> mLightCubeMap;
		std::unordered_map<LightComponentInstance*, uint> mLightFlagsMap;

		std::unique_ptr<Sampler2DArray> mSampler2DResource;
		std::unique_ptr<SamplerCubeArray> mSamplerCubeResource;
		std::unique_ptr<DepthRenderTexture2D> mShadowTextureDummy;

		bool mShadowResourcesCreated = false;

		// Cube maps from file
		std::unique_ptr<CubeRenderTarget>			mCubeMapFromFileRenderTarget;

		std::unique_ptr<NoMesh>						mNoMesh;
		std::unique_ptr<MaterialInstanceResource>	mMaterialInstResource;
		std::unique_ptr<MaterialInstance>			mMaterialInstance;				///< The MaterialInstance as created from the resource. 
		Material*									mCubeMapMaterial = nullptr;

		std::vector<std::unique_ptr<CubeRenderTarget>> mCubeMapFromFileTargets;

		static constexpr const uint mMaxShadowMapCount = 8;
		static constexpr const uint mRequiredVulkanVersionMajor = 1U;
		static constexpr const uint mRequiredVulkanVersionMinor = 0U;
	};
}
