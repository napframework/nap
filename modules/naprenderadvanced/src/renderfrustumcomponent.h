/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <renderablemeshcomponent.h>
#include <mesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>

// Local includes
#include "boxframemesh.h"

namespace nap
{
	// Forward declares
	class RenderFrustumComponentInstance;
	class CameraComponentInstance;

	/**
	 * RenderFrustumComponent
	 */
	class NAPAPI RenderFrustumComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderFrustumComponent, RenderFrustumComponentInstance)

	public:
		/**
		 * RenderFrustumComponent requires a camera whose frustum to render
		 * @param components the components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * RenderFrustumComponentInstance
	 */
	class NAPAPI RenderFrustumComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		// Default constructor
		RenderFrustumComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * onDraw() override
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Returns the resource associated with this instance
		 */
		RenderFrustumComponent& getResource();

	private:
		bool updateFrustum(utility::ErrorState& errorState);

		RenderFrustumComponent*						mResource = nullptr;
		RenderService*								mRenderService = nullptr;
		CameraComponentInstance*					mCamera = nullptr;

		std::unique_ptr<BoxFrameMesh>				mFrustumMesh;
	};
}
