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
	 * Renders an effect directly into a texture without having to define a render target or mesh.
	 * Use this component as a post process render step.
	 * This component manages its own render target and plane to render to.
	 * The plane is automatically scaled to fit the bounds of the output texture.
	 * Resource part of the component.
	 */
	class NAPAPI RenderToTextureComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderToTextureComponent, RenderToTextureComponentInstance)
	public:
		ResourcePtr<RenderTexture2D>	mOutputTexture = nullptr;					///< Property: 'OutputTexture' the target of the render step
		MaterialInstanceResource		mMaterialInstanceResource;					///< Property: 'MaterialInstance' instance of the material, used to override uniforms for this instance
		RGBColor8						mClearColor = { 255, 255, 255 };			///< Property: 'ClearColor' the color that is used to clear the render target
		std::string						mProjectMatrixUniform = "projectionMatrix";	///< Property: 'ProjectionMatrixUniform' name of the projection matrix uniform in the shader.
		std::string						mModelMatrixUniform = "modelMatrix";		///< Property: 'ModelMatrixUniform' name of the model matrix uniform in the shader.
	};


	/**
	 * Renders an effect directly into a texture without having to define a render target or mesh.
	 * Use this component as a post process render step.
	 * This component manages its own render target and plane to render to.
	 * The plane is automatically scaled to fit the bounds of the output texture.
	 * Simply declare the component in json and call draw() in the render part of your application.
	 * It is still possible to render this component through the render service, although only orthographic cameras are supported.
	 */
	class NAPAPI RenderToTextureComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		RenderToTextureComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize RenderToTextureComponentInstance based on the RenderToTextureComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the rendertotexturecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update rendertotexturecomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @return the render target that is used to perform the render step	
		 */
		IRenderTarget& getTarget();

		/**
		 * @return the output texture
		 */
		Texture2D& getOutputTexture(); 

		/**
		 * Directly executes the render step without having to go through the render service.
		 * Call this in your app render() call. 
		 * The render target associated with this component is automatically cleared and bound.
		 * Simply cal draw() and the result is rendered into the current output texture.
		 * A custom orthographic projection matrix is constructed based on the size of the render target.
		 * Alternatively, you can use the render service to render this component, see onDraw()
		 */
		void draw();

		/**
		 * Called by the Render Service.
		 * Only orthographic cameras are supported when rendering through the render service!
		 */
		virtual bool isSupported(nap::CameraComponentInstance& camera) const override;

		/**
		 * @return current material used when drawing the mesh.
		 */
		MaterialInstance& getMaterialInstance();

	protected:
		/**
		* Draws the plane full screen to the currently active render target.
		* @param viewMatrix often the camera world space location
		* @param projectionMatrix often the camera projection matrix
		*/
		virtual void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		nap::RenderTarget	mTarget;										///< Internally managed render target
		nap::PlaneMesh		mPlane;											///< Plane used for rendering the effect onto
		MaterialInstance	mMaterialInstance;								///< The MaterialInstance as created from the resource.
		RenderableMesh		mRenderableMesh;								///< Valid Plane / Material combination
		RenderService*		mService = nullptr;								///< Render service
		glm::mat4x4			mModelMatrix;									///< Plane model matrix
		bool				mDirty = true;									///< If the model matrix needs to be recomputed
		std::string			mModelMatrixUniform;							///< Name of the model matrix uniform in the shader
		std::string			mProjectMatrixUniform;							///< Name of the projection matrix uniform in the shader
	
		/**
		 * Checks if the uniform is available on the source material
		 * @return if the uniform is available or not
		 */
		bool ensureUniform(const std::string& uniformName, utility::ErrorState& error);

		/**
		 * Computes the model matrix based on current frame buffer size.
		 * The model matrix if only computed if the output texture is set or changed.
		 */
		void computeModelMatrix();
	};
}
