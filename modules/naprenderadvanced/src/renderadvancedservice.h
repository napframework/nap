/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <depthrendertarget.h>

// Local includes
#include "lightcomponent.h"

namespace nap
{
	// Forward declares
	class RenderableComponentInstance;


	//////////////////////////////////////////////////////////////////////////
	// Render Advanced Service
	//////////////////////////////////////////////////////////////////////////

	class NAPAPI RenderAdvancedServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:
		virtual rtti::TypeInfo getServiceType() const;

		/**
		 * Shadow map size
		 */
		uint mShadowMapSize = 1024;
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
		 *
		 */
		virtual void preShutdown() override;

		/**
		 * Render shadows
		 */
		void renderShadows(const std::vector<RenderableComponentInstance*>& comps);

	protected:
		//void registerObjectCreators(rtti::Factory& factory) override;

		void registerLightComponent(LightComponentInstance& light);

	private:
		// Registered light component instances
		std::vector<LightComponentInstance*> mLightComponents;

		// Shadow map
		std::unique_ptr<DepthRenderTexture2D> mShadowMapTexture;
		std::unique_ptr<DepthRenderTarget> mShadowMapTarget;
	};
}
