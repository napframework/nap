/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External includes
#include <rendercomponent.h>
#include <nap/resourceptr.h>
#include <boxmesh.h>
#include <materialinstance.h>
#include <renderablemesh.h>
#include <transformcomponent.h>

namespace nap
{
	// Forward declares
	class RenderSkyBoxComponentInstance;

	/**
	 * Resource part of RenderSkyBoxComponent
	 *
	 * Renders a skybox using a cube texture. Creates a `nap::BoxMesh` with front-face
	 * culling enabled internally. The default skybox shader negates the translational component of the view matrix
	 * to fake unlimited depth. This object should be rendered first to fill the background; to do this, ensure it is in
	 * the back layer. You may also want to exclude this object from a shadow rendering pass using tags. The shader
	 * variables are set automatically from this component's properties.
	 */
	class NAPAPI RenderSkyBoxComponent : public RenderableComponent
	{
		RTTI_ENABLE(RenderableComponent)
		DECLARE_COMPONENT(RenderSkyBoxComponent, RenderSkyBoxComponentInstance)

	public:

		// Depends on a transform component
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<TextureCube> mCubeTexture;
		RGBColorFloat mColor = { 1.0f, 1.0f, 1.0f };	///< Sky box color, defaults to white
		float mOpacity = 1.0f;							///< Sky box alpha, only available when 'BlendMode' is set to 'AlphaBlend'
		EBlendMode mBlendMode = EBlendMode::Opaque;		///< Sky box blend mode, change to 'AlphaBlend' to enable opacity
	};


	/**
	 * Instance part of RenderSkyBoxComponent
	 *
	 * Renders a skybox using a cube texture. Creates a `nap::BoxMesh` with front-face
	 * culling enabled internally. The default skybox shader negates the translational component of the view matrix
	 * to fake unlimited depth. This object should be rendered first to fill the background; to do this, ensure it is in
	 * the back layer. You may also want to exclude this object from a shadow rendering pass using tags. The shader
	 * variables are set automatically from this component's properties.
	 */
	class NAPAPI RenderSkyBoxComponentInstance : public RenderableComponentInstance
	{
		RTTI_ENABLE(RenderableComponentInstance)
	public:
		// Default constructor
		RenderSkyBoxComponentInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initializes the skybox render component, including required materials, meshes and bindings.
		 * @param errorState contains the error when initialization fails
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Update the sky box cube texture
		 * @param texture the new skybox texture
		 */
		void setTexture(const TextureCube& texture);

		/**
		 * Get the current sky box texture
		 * @return current sky box texture
		 */
		const TextureCube& getTexture() const					{ return mCubeSampler->getTexture(); }

		/**
		 * Set the sky box color
		 * @param color new skybox color
		 */
		void setColor(const glm::vec3& color)					{ mColorUniform->setValue(color); }

		/**
		 * @return sky box color
		 */
		const glm::vec3& getColor() const						{ return mColorUniform->getValue(); }

		/**
		 * Set the sky box opacity
		 * @param opacity new sky box opacity
		 */
		void setOpacity(float opacity)							{ mAlphaUniform->setValue(opacity); }

		/**
		 * @return sky box opacity
		 */
		float getOpacity() const								{ return mAlphaUniform->getValue(); }

	protected:
		/**
		 * Draws the skybox to the given render target
		 */
		void onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		RenderSkyBoxComponent*		mResource = nullptr;
		RenderService&				mRenderService;

		BoxMesh						mSkyBoxMesh;					///< Skybox mesh
		MaterialInstance			mMaterialInstance;				///< The MaterialInstance as created from the resource.
		MaterialInstanceResource	mMaterialInstanceResource;		///< Resource used to initialize the material instance
		RenderableMesh				mRenderableMesh;				///< Renderable skybox mesh / material combo
		TransformComponentInstance* mTransformComponent;			///< Cached pointer to transform

		UniformStructInstance*		mMVPStruct = nullptr;			///< model view projection struct
		UniformMat4Instance*		mModelMatUniform = nullptr;		///< Pointer to the model matrix uniform
		UniformMat4Instance*		mViewMatUniform = nullptr;		///< Pointer to the view matrix uniform
		UniformMat4Instance*		mProjectMatUniform = nullptr;	///< Pointer to the projection matrix uniform
		UniformFloatInstance*		mAlphaUniform = nullptr;		///< Pointer to the alpha uniform
		UniformVec3Instance*		mColorUniform = nullptr;		///< Pointer the color uniform
		SamplerCubeInstance*		mCubeSampler = nullptr;			///< Pointer to the cube sampler
	};
}
