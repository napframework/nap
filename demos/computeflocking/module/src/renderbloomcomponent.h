/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rendertotexturecomponent.h"
#include "blurshader.h"

namespace nap
{
	// Forward Declares
	class RenderBloomComponentInstance;

	/**
	 * Renders to a texture
	 */
	class NAPAPI RenderBloomComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderBloomComponent, RenderBloomComponentInstance)
	public:
		ResourcePtr<RenderTexture2D>	mInputTexture = nullptr;							///< Property: 'InputTexture' the input color texture
		EBlurSamples					mKernel = EBlurSamples::X5;							///< Property: 'Kernel' the blur kernel
		uint							mPassCount = 1;										///< Property: 'PassCount' the number of combined horizontal/vertical passes
	};


	/**
	 * Renders to a texture
	 */
	class NAPAPI RenderBloomComponentInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderBloomComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderToTextureComponentInstance based on the RenderToTextureComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendertotexturecomponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * 
		 */
		void draw();

		/**
		 * 
		 */
		Texture2D& getOutputTexture() { return *mBloomRTs.back()[1]->mColorTexture; }

	protected:
		/**
		 * Draws the effect full screen to the currently active render target,
		 * when the view matrix = identity.
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		using DoubleBufferedRenderTarget = std::array<rtti::ObjectPtr<RenderTarget>, 2>;

		RenderService*				mRenderService = nullptr;			///< Render service
		RenderTexture2D*			mResolvedInputTexture = nullptr;	///<
		
		std::vector<DoubleBufferedRenderTarget> mBloomRTs;				///< Internally managed render targets

		MaterialInstanceResource	mMaterialInstanceResource;			///< Instance of the material, used to override uniforms for this instance
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.

		RenderableMesh				mRenderableMesh;					///< Valid Plane / Material combination
		nap::PlaneMesh				mPlane;								///< Plane used for rendering the effect onto

		glm::mat4x4					mModelMatrix;						///< Plane model matrix

		UniformMat4Instance*		mModelMatrixUniform = nullptr;		///< Name of the model matrix uniform in the shader
		UniformMat4Instance*		mProjectMatrixUniform = nullptr;	///< Name of the projection matrix uniform in the shader
		UniformMat4Instance*		mViewMatrixUniform = nullptr;		///< View matrix uniform
		UniformStructInstance*		mMVPStruct = nullptr;				///< model view projection struct

		Sampler2DInstance*			mColorTextureSampler = nullptr;
		UniformVec2Instance*		mDirectionUniform = nullptr;
		UniformVec2Instance*		mTextureSizeUniform = nullptr;

		/**
		 * Checks if the uniform is available on the source material and creates it if so
		 * @return the uniform, nullptr if not available.
		 */
		UniformMat4Instance* ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error);
	};
}
