/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rendercomponent.h"
#include "rendertexture2d.h"
#include "planemesh.h"
#include "rendertarget.h"
#include "materialinstance.h"
#include "renderablemesh.h"
#include "color.h"

// External Includes
#include <nap/resourceptr.h>
#include <entity.h>

namespace nap
{
	// Forward Declares
	class RenderToTextureComponentInstance;

	/**
	 * Renders an effect directly to a texture using a custom material without having to define a render target or mesh.
	 * Use this component as a post process render step.
	 * This component manages its own render target and plane to render to.
	 * The plane is automatically scaled to fit the bounds of the render target.
	 *
	 * Simply declare the component in json and call RenderToTextureComponentInstance::draw() in the render part of your application,
	 * in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
	 *
	 * This component expects a material with a shader that contains both a model and projection matrix uniform.
	 * The view matrix uniform is optional. It will be set if found, otherwise bypassed.
	 * If you don't care about view space (camera) transformation, don't declare it.
	 *
	 * ~~~~~
	 *	uniform nap
	 *	{
	 *		uniform mat4 projectionMatrix;
	 *		uniform mat4 modelMatrix;
	 *	} mvp;
	 *	...
	 *	void main(void)
	 *	{
	 *		gl_Position = mvp.projectionMatrix * mvp.modelMatrix;
	 *	}
	 * ~~~~~
	 */
	class NAPAPI RenderToTextureComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderToTextureComponent, RenderToTextureComponentInstance)
	public:
		bool							mSampleShading = true;								///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		ResourcePtr<RenderTexture2D>	mOutputTexture = nullptr;							///< Property: 'OutputTexture' the target of the render step
		MaterialInstanceResource		mMaterialInstanceResource;							///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance
		RGBColor8						mClearColor = { 255, 255, 255 };					///< Property: 'ClearColor' the color that is used to clear the render target
	};


	/**
	 * Renders an effect directly to a texture using a custom material without having to define a render target or mesh.
	 * Use this component as a post process render step.
	 * This component manages its own render target and plane to render to.
	 * The plane is automatically scaled to fit the bounds of the render target.
	 *
	 * Simply declare the component in json and call RenderToTextureComponentInstance::draw() in the render part of your application,
	 * in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
	 * It is still possible to render this component through the render service, although only orthographic cameras are supported.
	 *
	 * This component expects a material with a shader that contains both a model and projection matrix uniform.
	 * The view matrix uniform is optional. It will be set if found, otherwise bypassed.
	 * If you don't care about view space (camera) transformation, don't declare it in the shader.
	 * for example:
	 *
	 * ~~~~~
	 *	uniform nap
	 *	{
	 *		uniform mat4 projectionMatrix;
	 *		uniform mat4 modelMatrix;
	 *	} mvp;
	 *	...
	 *	void main(void)
	 *	{
	 *		gl_Position = mvp.projectionMatrix * mvp.modelMatrix;
	 *	}
	 * ~~~~~
	 *
	 */
	class NAPAPI RenderToTextureComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderToTextureComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderToTextureComponentInstance based on the RenderToTextureComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendertotexturecomponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the render target that is used to perform the render step	
		 */
		IRenderTarget& getTarget();

		/**
		 * @return the output texture
		 */
		Texture2D& getOutputTexture(); 

		/**
		 * Renders the effect directly to texture using a custom material, without having to define a render target or mesh.
		 * Call this in your application render() call, 
		 * in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
		 * Do not call this function outside of a headless recording pass, ie: when rendering to a window.
		 * The result is rendered into the given output texture. 
		 * Alternatively, you can use the render service to render this component, see onDraw()
		 */
		void draw();

		/**
		 * Called by the Render Service. Only orthographic cameras are supported.
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override;

		/**
		 * @return current material used when drawing the mesh.
		 */
		MaterialInstance& getMaterialInstance();

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
		nap::RenderTarget			mTarget;							///< Internally managed render target
		nap::PlaneMesh				mPlane;								///< Plane used for rendering the effect onto
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.
		RenderableMesh				mRenderableMesh;					///< Valid Plane / Material combination
		RenderService*				mService = nullptr;					///< Render service
		glm::mat4x4					mModelMatrix;						///< Plane model matrix
		bool						mDirty = true;						///< If the model matrix needs to be recomputed
		UniformMat4Instance*		mModelMatrixUniform = nullptr;		///< Name of the model matrix uniform in the shader
		UniformMat4Instance*		mProjectMatrixUniform = nullptr;	///< Name of the projection matrix uniform in the shader
		UniformMat4Instance*		mViewMatrixUniform = nullptr;		///< View matrix uniform
		UniformStructInstance*		mMVPStruct = nullptr;				///< model view projection struct

		/**
		 * Checks if the uniform is available on the source material and creates it if so
		 * @return the uniform, nullptr if not available.
		 */
		UniformMat4Instance* ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error);
	};
}
