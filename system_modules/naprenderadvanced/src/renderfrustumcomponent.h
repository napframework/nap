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
	 * usage `EMemoryUsage::DynamicWrite`. Supports `nap::PerspCameraComponent` and `nap::OrthoCameraComponent`.
	 * As the box frame mesh only has a position attribute this component is rendered with a simple `nap::ConstantShader`.
	 */
	class NAPAPI RenderFrustumComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderFrustumComponent, RenderFrustumComponentInstance)

	public:
		/**
		 * RenderFrustumComponent requires a camera whose frustum to render
		 * @param components the components this component depends upon.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float mLineWidth = 2.0f;							///< Property: 'LineWidth' frustrum line width
		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };		///< Property: 'Color' frustrum draw color
		float mOpacity = 1.0f;								///< Property: 'Opacity' frustrum alpha
	};


	/**
	 * Instance part of RenderFrustumComponent
	 *
	 * Renders a camera frustum to aid in visual debugging. The is a dynamic line mesh that is updated on the CPU to match
	 * the latest camera properties e.g. field of view. Ignores the `Mesh` property and creates a `nap::BoxFrameMesh` with
	 * usage `EMemoryUsage::DynamicWrite`. Supports `nap::PerspCameraComponent` and `nap::OrthoCameraComponent`.
	 * As the box frame mesh only has a position attribute this component is rendered with a simple `nap::ConstantShader`.
	 */
	class NAPAPI RenderFrustumComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		// Default constructor
		RenderFrustumComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return frustrum draw color
		 */
		const glm::vec3& getColor() const													{ return mColorUniform->getValue(); }

		/**
		 * Set the frustrum draw color
		 */
		void setColor(const glm::vec3& color)												{ mColorUniform->setValue(color); }

		/**
		 * @return frustrum draw opacity
		 */
		float getOpacity() const															{ return mAlphaUniform->getValue(); }

		/**
		 * Set the frustrum draw opacity
		 */
		void setOpacity(float opacity)														{ mAlphaUniform->setValue(opacity); }

		/**
		 * Called by the Render Service. Supports orthographic and perspective cameras
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override		{ return true; }

	protected:
		/**
		 * Draws the current camera frustrum
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		/**
		 * Updates the frustrum mesh based on current camera settings
		 * @param errorState contains the error if the frustrum fails to update
		 * @return if the frustrum updated correct
		 */
		bool updateFrustum(utility::ErrorState& errorState);

		RenderFrustumComponent* mResource = nullptr;
		RenderService* mRenderService = nullptr;
		CameraComponentInstance* mCamera = nullptr;
		TransformComponentInstance* mCameraTransform = nullptr;

		BoxFrameMesh mFrustumMesh;													///< The frustrum mesh
		RenderableMesh mRenderableMesh;												///< The renderable frustrum mesh
		MaterialInstance mMaterialInstance;											///< The MaterialInstance as created from the resource.
		MaterialInstanceResource mMaterialInstanceResource;							///< Resource used to initialize the material instance

		UniformStructInstance* mMVPStruct = nullptr;								///< model view projection struct
		UniformMat4Instance* mModelMatUniform = nullptr;							///< Pointer to the model matrix uniform
		UniformMat4Instance* mViewMatUniform = nullptr;								///< Pointer to the view matrix uniform
		UniformMat4Instance* mProjectMatUniform = nullptr;							///< Pointer to the projection matrix uniform

		UniformStructInstance* mUBOStruct = nullptr;								///< UBO struct
		UniformVec3Instance* mColorUniform = nullptr;								///< Constant color uniform
		UniformFloatInstance* mAlphaUniform = nullptr;								///< Alpha uniform
	};
}

