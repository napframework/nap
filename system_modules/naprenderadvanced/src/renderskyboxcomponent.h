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
	 * Resource part of RenderSkyBoxComponent
	 *
	 * Renders a skybox from a cube texture. Should be used with a `nap::SkyBoxShader` and `nap::BoxMesh` in the material
	 * resource, but custom implementations are possible. The default sjybox shader negates the translational component of
	 * the view matrix to fake unlimited depth. This object should be rendered first to fill the background; to do this,
	 * ensure it is in the back layer. You may also want to exclude this object from a shadow rendering pass using tags.
	 * The shader variables are set automatically from this component's properties.
	 */
	class NAPAPI RenderSkyBoxComponent : public RenderableMeshComponent
	{
		RTTI_ENABLE(RenderableMeshComponent)
		DECLARE_COMPONENT(RenderSkyBoxComponent, RenderSkyBoxComponentInstance)

	public:
		ResourcePtr<TextureCube> mCubeTexture;
		ResourcePtr<ParameterEntryRGBColorFloat> mColor;
	};


	/**
	 * Instance part of RenderSkyBoxComponent
	 *
	 * Renders a skybox from a cube texture. Should be used with a `nap::SkyBoxShader` and `nap::BoxMesh` in the material
	 * resource, but custom implementations are possible. The default sjybox shader negates the translational component of
	 * the view matrix to fake unlimited depth. This object should be rendered first to fill the background; to do this,
	 * ensure it is in the back layer. You may also want to exclude this object from a shadow rendering pass using tags.
	 * The shader variables are set automatically from this component's properties.
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
