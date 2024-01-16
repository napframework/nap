/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rendercomponent.h>
#include <componentptr.h>
#include <perspcameracomponent.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <planemesh.h>
#include <parameternumeric.h>

namespace nap
{
	// Forward Declares
	class RenderDOFComponentInstance;

	/**
	 * Pre- or post-processing effect that blurs the input texture at a downsampled resolution into internally managed
	 * rendertargets.
	 *
	 * Resource-part of RenderDOFComponentInstance.
	 */
	class NAPAPI RenderDOFComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderDOFComponent, RenderDOFComponentInstance)
	public:
		ComponentPtr<PerspCameraComponent>	mCamera;										///< Property: 'Camera'
		ResourcePtr<RenderTarget>			mInputTarget;									///< Property: 'InputTarget' the input color target, must be copyable
		ResourcePtr<RenderTexture2D>		mOutputTexture;									///< Property: 'OutputTexture' the output color texture
		uint								mPassCount = 1;									///< Property: 'PassCount' the number of combined horizontal/vertical passes

		ResourcePtr<ParameterFloat> mAperture;
		ResourcePtr<ParameterFloat> mFocusPower;
		ResourcePtr<ParameterFloat> mFocusDistance;

	};


	/**
	 * Pre- or post-processing effect that blurs the input texture at a downsampled resolution into internally managed
	 * rendertargets.
	 *
	 * This component manages its own render target and plane to render to.
	 * The plane is automatically scaled to fit the bounds of the render target.
	 *
	 * `InputTexture` is blitted to an internally managed render target, and then blurred based on the specified pass
	 * count. Each blur 'pass' then comprises two passes; horizontal and vertical. The gaussian sampling kernel size can
	 * be specified with the 'Kernel' property. Each subsequent pass performs a horizontal and vertical blur at half the
	 * resolution of the former pass, with the first being at half the resolution of `InputTexture`.
	 * 
	 * When the bloom passes have been completed, the result is blitted to `OutputTexture`. `InputTexture` and
	 * `OutputTexture` are allowed to refer to the same `nap::RenderTexture`.
	 *
	 * Simply declare the component in json and call RenderDOFComponentInstance::draw() in the render part of your
	 * application, in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
	 *
	 * This component uses the default naprender blur shader and automatically sets its shader variables based on this
	 * component's properties configuration. 
	 */
	class NAPAPI RenderDOFComponentInstance : public  RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderDOFComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderDOFComponentInstance based on the RenderDOFComponent resource.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the RenderDOFComponentInstance initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Renders the effect to the output texture, without having to define a render target or mesh.
		 * Call this in your application render() call inbetween nap::RenderService::beginHeadlessRecording()
		 * and nap::RenderService::endHeadlessRecording().
		 * Do not call this function outside of a headless recording pass i.e. when rendering to a window.
		 * The result is rendered into a dynamically created output texture, which is accessible through
		 * RenderDOFComponentInstance::getOutputTexture().
		 * Alternatively, you can use the render service to render this component, see onDraw().
		 */
		void draw();

		/**
		 * Returns the output texture with the bloom effect applied.
		 * The size of this texture equals { input_width/2^PassCount, input_height/2^PassCount }
		 * @return the bloom texture created from the specified input texture
		 */
		Texture2D& getOutputTexture() { return *mOutputTexture; }

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

		/**
		 * Link to camera, can be used to make the copied meshes look at the camera
		 */
		ComponentInstancePtr<PerspCameraComponent> mCamera = { this, &RenderDOFComponent::mCamera };

		RenderDOFComponent*			mResource = nullptr;
		RenderService*				mRenderService = nullptr;			///< Render service
		RenderTarget*				mInputTarget = nullptr;				///< Reference to the input target
		RenderTexture2D*			mOutputTexture = nullptr;			///< Reference to the output texture
		
		std::vector<DoubleBufferedRenderTarget> mBloomRTs;				///< Internally managed render targets

		MaterialInstanceResource	mMaterialInstanceResource;			///< Instance of the material, used to override uniforms for this instance
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.

		RenderableMesh				mRenderableMesh;					///< Valid Plane / Material combination
		nap::PlaneMesh				mPlane;								///< Plane used for rendering the effect onto

		glm::mat4x4					mModelMatrix;						///< Plane model matrix

		UniformMat4Instance*		mModelMatrixUniform = nullptr;		///< Name of the model matrix uniform in the shader
		UniformMat4Instance*		mProjectMatrixUniform = nullptr;	///< Name of the projection matrix uniform in the shader
		UniformMat4Instance*		mViewMatrixUniform = nullptr;		///< View matrix uniform
		UniformStructInstance*		mMVPStruct = nullptr;				///< Model View Projection struct

		Sampler2DInstance*			mColorTextureSampler = nullptr;		///< Sampler instance for color textures in the blur material
		Sampler2DInstance*			mDepthTextureSampler = nullptr;		///< Sampler instance for depth textures in the blur material
		UniformVec2Instance*		mDirectionUniform = nullptr;		///< Direction uniform of the blur material
		UniformVec2Instance*		mTextureSizeUniform = nullptr;		///< Texture size uniform of the blur material
		UniformVec2Instance*		mNearFarUniform = nullptr;			///< 
		UniformFloatInstance*		mApertureUniform = nullptr;			///< 
		UniformFloatInstance*		mFocusPowerUniform = nullptr;		///< 
		UniformFloatInstance*		mFocusDistanceUniform = nullptr;	///< 

		/**
		 * Checks if the uniform is available on the source material and creates it if so
		 * @return the uniform, nullptr if not available.
		 */
		UniformMat4Instance* ensureUniform(const std::string& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error);
	};
}
