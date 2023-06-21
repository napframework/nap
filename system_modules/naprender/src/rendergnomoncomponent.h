/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "gnomonmesh.h"
#include "rendercomponent.h"
#include "materialinstance.h"

// External Includes
#include <nap/resourceptr.h>
#include <transformcomponent.h>

namespace nap
{
	class RenderGnomonComponentInstance;

	/**
	 * Renders a nap::GnomonMesh using a hard-coded nap::GnomonShader to target.
	 * The Gnomon material is automatically created on initialization.
	 * A Transform component is required to position the Gnomon.
	 */
	class NAPAPI RenderGnomonComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderGnomonComponent, RenderGnomonComponentInstance)
	public:

		ResourcePtr<GnomonMesh>	mMesh;					///< Property: 'Gnomon' the Gnomon mesh this component renders.
		float mLineWidth = 1.0f;						///< Property: 'LineWidth' Width of the line when rendered, values higher than 1.0 only work when the GPU supports it.
		EDepthMode mDepthMode = EDepthMode::ReadWrite;	///< Property: 'DepthMode' Used depth mode.

		/**
		 * Requires a transform to position itself in the world.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * Renders a nap::GnomonMesh using a hard-coded nap::GnomonShader to target.
	 * The Gnomon material is automatically created on initialization.
	 * A Transform component is required to position the Gnomon.
	 */
	class NAPAPI RenderGnomonComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderGnomonComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableComponentInstance(entity, resource)									{ }

		/**
		 * Draws the Gnomon to the currently active render target.
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

		/**
		 * Called by the Render Service. Supports orthographic and perspective cameras
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override		{ return true; }

		/**
		 * Initialize based on resource
		 * @param errorState holds the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		MaterialInstance			mMaterialInstance;				///< The MaterialInstance as created from the resource.
		MaterialInstanceResource	mMaterialInstanceResource;		///< Resource used to initialize the material instance
		RenderableMesh				mRenderableMesh;				///< The origin mesh / material that is rendered to target
		RenderService*				mRenderService = nullptr;		///< The render engine
		TransformComponentInstance*	mTransformComponent = nullptr;	///< The transform component
		float						mLineWidth = 1.0f;				///< Line width
		UniformStructInstance*		mMVPStruct = nullptr;			///< model view projection struct
		UniformMat4Instance*		mModelMatUniform = nullptr;		///< Pointer to the model matrix uniform
		UniformMat4Instance*		mViewMatUniform = nullptr;		///< Pointer to the view matrix uniform
		UniformMat4Instance*		mProjectMatUniform = nullptr;	///< Pointer to the projection matrix uniform

		/**
		 * Checks if the uniform is available on the source material and creates it if so
		 * @return the uniform, nullptr if not available.
		 */
		UniformMat4Instance* ensureUniform(const std::string& uniformName, utility::ErrorState& error);
	};
}
