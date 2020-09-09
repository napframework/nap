#pragma once

// Local Includes
#include "videoplayer.h"

// External Includes
#include <component.h>
#include <rendercomponent.h>
#include <nap/resourceptr.h>
#include <rendertexture2d.h>
#include <planemesh.h>
#include <rendertarget.h>
#include <color.h>
#include <materialinstance.h>
#include <renderablemesh.h>

namespace nap
{
	// Forward Declares
	class RenderVideoToTextureComponentInstance;

	/**
	 * rendervideototexturecomponent
	 */
	class NAPAPI RenderVideoToTextureComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderVideoToTextureComponent, RenderVideoToTextureComponentInstance)
	public:

		ResourcePtr<VideoPlayer>		mVideoPlayer = nullptr;				///< Property: 'VideoPlayer' the video player to render to texture
		ResourcePtr<RenderTexture2D>	mOutputTexture = nullptr;			///< Property: 'OutputTexture' the RGB8 texture to render output to
		RGBColor8						mClearColor = { 255, 255, 255 };	///< Property: 'ClearColor' the color that is used to clear the render target
	};


	/**
	 * rendervideototexturecomponentInstance	
	 */
	class NAPAPI RenderVideoToTextureComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderVideoToTextureComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize rendervideototexturecomponentInstance based on the rendervideototexturecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendervideototexturecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Called by the Render Service.
		 * Only orthographic cameras are supported when rendering through the render service!
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override;

		/**
		 * Returns the rendered RGB video texture.
		 * @return the rendered RGB video texture.
		 */
		Texture2D& getOutputTexture();

		/**
		* Directly render the video without having to go through the render service.
		* Call this in your application render() call,
		* in between nap::RenderService::beginHeadlessRecording() and nap::RenderService::endHeadlessRecording().
		* Do not call this function outside of a headless recording pass, ie: when rendering to a window.
		* The result is rendered into the given output texture.
		* A custom orthographic projection matrix is constructed based on the size of the render target.
		* Alternatively, you can use the render service to render this component, see onDraw()
		*/
		void draw();

	protected:
		/**
		* Draws the data to the currently active render target.
		* Override this method to implement your own custom draw behavior.
		* This method won't be called if the mesh isn't visible!
		* @param viewMatrix often the camera world space location
		* @param projectionMatrix often the camera projection matrix
		*/
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		VideoPlayer*				mPlayer = nullptr;								///< Video player to render	
		RenderTexture2D*			mOutputTexture = nullptr;						///< Texture currently bound by target
		RGBColorFloat				mClearColor = { 0.0f, 0.0f, 0.0f };				///< Target Clear Color
		RenderTarget				mTarget;										///< Target video is rendered into
		PlaneMesh					mPlane;											///< Plane that is rendered
		MaterialInstance			mMaterialInstance;								///< The MaterialInstance as created from the resource.
		MaterialInstanceResource	mMaterialInstanceResource;						///< Resource used to initialize the material instance
		RenderableMesh				mRenderableMesh;								///< Valid Plane / Material combination
		RenderService*				mRenderService = nullptr;						///< Pointer to the render service
		VideoService*				mVideoService = nullptr;						///< Pointer to the video service
		UniformMat4Instance*		mModelMatrixUniform = nullptr;					///< Model matrix uniform in the material
		UniformMat4Instance*		mProjectMatrixUniform = nullptr;				///< Projection matrix uniform in the material
		UniformMat4Instance*		mViewMatrixUniform = nullptr;					///< View matrix uniform in the material
		UniformStructInstance*		mMVPStruct = nullptr;							///< model view projection struct
		Sampler2DInstance*			mYSampler = nullptr;							///< Video material Y sampler
		Sampler2DInstance*			mUSampler = nullptr;							///< Video material U sampler
		Sampler2DInstance*			mVSampler = nullptr;							///< Video material V sampler
		glm::mat4x4					mModelMatrix;									///< Computed model matrix, used to scale plane to fit target bounds
		bool						mDirty = true;									///< If the model matrix needs to be re-computed

		/**
		 * Checks if the uniform is available on the source material and creates it if so
		 * @return the uniform, nullptr if not available.
		 */
		UniformMat4Instance* ensureUniform(const std::string& uniformName, utility::ErrorState& error);

		/**
		 * Checks if the sampler with the given name is available on the source material and creates it if so
		 * @return new or created sampler
		 */
		Sampler2DInstance* ensureSampler(const std::string& samplerName, utility::ErrorState& error);

		/**
		 * Computes the model matrix based on current frame buffer size.
		 * The model matrix if only computed if the output texture is set or changed.
		 */
		void computeModelMatrix();

		/**
		 * Called every time a new video is selected.
		 * Updates the YUV textures in the video material.
		 * @param player the video player that switched the video.
		 */
		void videoChanged(VideoPlayer& player);

		// Called when video selection changes
		nap::Slot<VideoPlayer&> mVideoChangedSlot = { this, &RenderVideoToTextureComponentInstance::videoChanged };
	};
}
