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
	 * Resource part of RenderFrustumComponent
	 * 
	 * Renders a camera frustum to aid in visual debugging. The is a dynamic line mesh that is updated on the CPU to match
	 * the latest camera properties e.g. field of view. Ignores the `Mesh` property and creates a `nap::BoxFrameMesh` with
	 * usage `EMemoryUsage::DynamicWrite` and polygon mode `EPolygonMode::Line`. Supports `nap::PerspCameraComponent` and
	 * `nap::OrthoCameraComponent`. As the box frame mesh only has a position attribute this component must be rendered
	 * with a simple shader such as `nap::ConstantShader`.
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
	 * Instance part of RenderFrustumComponent
	 *
	 * Renders a camera frustum to aid in visual debugging. The is a dynamic line mesh that is updated on the CPU to match
	 * the latest camera properties e.g. field of view. Ignores the `Mesh` property and creates a `nap::BoxFrameMesh` with
	 * usage `EMemoryUsage::DynamicWrite` and polygon mode `EPolygonMode::Line`. Supports `nap::PerspCameraComponent` and
	 * `nap::OrthoCameraComponent`. As the box frame mesh only has a position attribute this component must be rendered
	 * with a simple shader such as `nap::ConstantShader`.
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
