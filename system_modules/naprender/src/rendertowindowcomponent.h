#pragma once

// Local includes
#include "renderwindow.h"
#include "materialinstance.h"
#include "planemesh.h"
#include "renderablemesh.h"
#include "rendercomponent.h"

namespace nap
{
	class RenderToWindowComponentInstance;

	/**
	 * Renders a plane directly to a window using a custom material, centering and scaling it based on the selected mode.
	 * Use this component to, for example, present the output of a render-pass to screen.
	 *
	 * Simply declare the component in json and call RenderToWindowComponent::draw() in the render part of your application.
	 * This component manages its own nap::PlaneMesh to render with, you only have to provide it with a material.
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
	class NAPAPI RenderToWindowComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
			DECLARE_COMPONENT(RenderToWindowComponent, RenderToWindowComponentInstance)
	public:

		enum class EScaleMode : uint8
		{
			Window		= 0,	///< Canvas is scaled to fit window
			Square		= 1,	///< Canvas is squared (1:1) and scaled to fit window
			Custom		= 2		///< Canvas is scaled using the provided 'Ratio' and fit to window
		};

		ResourcePtr<RenderWindow> mWindow = nullptr;			///< Property: 'Window' the window target
		MaterialInstanceResource mMaterialInstanceResource;		///< Property: 'MaterialInstance' material instance
		EScaleMode mMode = EScaleMode::Window;					///< Property: 'ScaleMode' canvas scaling method
		glm::vec2 mRatio = { 1.0f, 1.0f };					///< Property: 'Ratio' ratio to use when mode is set to 'Custom'
	};


	/**
	 * Renders a plane directly to a window using a custom material, centering and scaling it based on the selected mode.
	 * Use this component to, for example, present the output of a render-pass to screen.
	 *
	 * Simply declare the component in json and call RenderToWindowComponent::draw() in the render part of your application.
	 * This component manages its own nap::PlaneMesh to render with, you only have to provide it with a material.
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
	class NAPAPI RenderToWindowComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RenderToWindowComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize the component
		 * @param errorState holds the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		bool init(utility::ErrorState& errorState) override;

		/**
		 * @return current material used when drawing the canvas to window.
		 */
		MaterialInstance& getMaterialInstance()							{ return mMaterialInstance; }

		/**
		 * @return current material used when drawing the canvas to window.
		 */
		const MaterialInstance& getMaterialInstance() const				{ return mMaterialInstance; }

		/**
		 * Renders directly to the window using a custom material, without having to define a mesh.
		 * Call this in your application render() call -> the result is rendered into the given window.
		 * This call starts and stops the render operation, you can't render anything else to the window after it's done.
		 * Alternatively, you can use the render service to render this component, see onDraw()
		 */
		void draw();

		/**
		 * Only orthographic cameras are supported.
		 * @return true if camera is orthographic
		 */
		bool isSupported(nap::CameraComponentInstance& camera) const override;

		/**
		 * @return current canvas scaling mode
		 */
		RenderToWindowComponent::EScaleMode getMode() const				{ return mMode; }

		/**
		 * Set current canvas scaling mode
		 * @param mode canvas scaling mode
		 */
		void setMode(RenderToWindowComponent::EScaleMode mode)			{ mMode = mode; }

		/**
		 * Returns current canvas scaling ratio.
		 * @return canvas source ratio
		 */
		glm::vec2 getRatio() const;

		/**
		 * Set and use custom canvas scaling ratio.
		 * @param ratio new canvas scaling ratio
		 */
		void setRatio(const glm::vec2& ratio);

	protected:
		/**
		 * Draws full screen to the currently active render window, when the view matrix = identity.
		 * @param renderTarget the target to render to.
		 * @param commandBuffer the currently active command buffer.
		 * @param viewMatrix often the camera world space location
		 * @param projectionMatrix often the camera projection matrix
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		nap::PlaneMesh				mPlane;								///< Plane used for rendering the effect onto
		MaterialInstance			mMaterialInstance;					///< The MaterialInstance as created from the resource.
		RenderableMesh				mRenderableMesh;					///< Valid Plane / Material combination
		RenderService*				mService = nullptr;					///< Render service
		glm::mat4x4					mModelMatrix;						///< Plane model matrix
		UniformMat4Instance*		mModelMatrixUniform = nullptr;		///< Name of the model matrix uniform in the shader
		UniformMat4Instance*		mProjectMatrixUniform = nullptr;	///< Name of the projection matrix uniform in the shader
		UniformMat4Instance*		mViewMatrixUniform = nullptr;		///< View matrix uniform
		UniformStructInstance*		mMVPStruct = nullptr;				///< model view projection struct
		glm::vec2					mRatio = { 1.0f, 1.0f };			///< Canvas aspect ratio
		nap::RenderWindow*			mWindow = nullptr;					///< Window to render to

		RenderToWindowComponent::EScaleMode mMode = RenderToWindowComponent::EScaleMode::Window;
	};
}
