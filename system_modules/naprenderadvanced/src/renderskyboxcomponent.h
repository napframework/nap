/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <renderablemeshcomponent.h>
#include <mesh.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <boxmesh.h>
#include <parameterentrycolor.h>

namespace nap
{
	// Forward declares
	class RenderSkyBoxComponentInstance;
	class CameraComponentInstance;

	/**
	 * RenderSkyBoxComponent
	 */
	class NAPAPI RenderSkyBoxComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderSkyBoxComponent, RenderSkyBoxComponentInstance)

	public:
		/**
		 * RenderSkyBoxComponent requires a camera whose frustum to render
		 * @param components the components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<TextureCube> mCubeTexture;
		ResourcePtr<ParameterEntryRGBColorFloat> mColor;
	};


	/**
	 * RenderSkyBoxComponentInstance
	 */
	class NAPAPI RenderSkyBoxComponentInstance : public RenderableMeshComponentInstance
	{
		RTTI_ENABLE(RenderableMeshComponentInstance)
	public:
		// Default constructor
		RenderSkyBoxComponentInstance(EntityInstance& entity, Component& resource);

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
		RenderSkyBoxComponent& getResource();

	private:
		RenderSkyBoxComponent* mResource = nullptr;
		RenderService& mRenderService;
	};
}
