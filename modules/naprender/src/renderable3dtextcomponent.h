#pragma once

// Local Includes
#include "renderabletextcomponent.h"

// External Includes
#include <component.h>

namespace nap
{
	class Renderable3DTextComponentInstance;

	/**
	 * Draws flat text in 3D space.
	 * Use this component when you want to render text at a specific location in the world
	 * Use the normalize toggle to render the text at the origin of the scene with a unit size of 1.
	 * When rendering in normalized mode the initial text is used to compute the normalization factor.
	 * This ensures that when changing text at runtime the size of the letters don't change as well.
	 * Use the Renderable2DTextComponent to draw text in screen (pixel) space with an orthographic camera.
	 */
	class NAPAPI Renderable3DTextComponent : public RenderableTextComponent
	{
		RTTI_ENABLE(RenderableTextComponent)
		DECLARE_COMPONENT(Renderable3DTextComponent, Renderable3DTextComponentInstance)
	public:
		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		bool mNormalize = true;		///< Property: 'Normalize' text is rendered at the origin with normalized bounds (-0.5,0.5)
	};


	/**
	 * Runtime version of the Renderable3DTextComponent.
	 * This component allows you to render a single line of text at a specific location in the world.
	 * 3D text can only be rendered using the render service, similar to how 3D meshes are rendered.
	 * The text can be transformed, scaled and rotated. It's best to render 3D text using a perspective camera.
	 * The font size directly influences the size of the text unless normalization is turned on.
	 * When normalization is turned on the text is rendered centered on the origin with -0.5-0,5 bounds.
	 */
	class NAPAPI Renderable3DTextComponentInstance : public RenderableTextComponentInstance
	{
		RTTI_ENABLE(RenderableTextComponentInstance)
	public:
		Renderable3DTextComponentInstance(EntityInstance& entity, Component& resource) :
			RenderableTextComponentInstance(entity, resource)								{ }

		/**
		 * Initialize Renderable3DTextComponentInstance based on the Renderable3DTextComponent resource
		 * @param errorState should hold the error message when initialization fails
		 * @return if the Renderable3DTextComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Enables or disables normalization. Normalized text is centered around the origin with approx -0.5 / 0.5 bounds
		 * The normalization factor is based on reference text which can be updated by calling: computeNormalizationFactor()
		 * Calling computeNormalizationFactor() ensures that the size of the letters don't change size at runtime.
		 * @param enable disable or enable normalization
		 * @param referenceText the text used to calculate the normalization factor, can be left empty when disabled
		 */
		void normalizeText(bool enable)													{ mNormalize = enable; }

		/**
		 * @return if the text is drawn normalized, normalized text is 1 unit long and centered around the origin
		 */
		bool isNormalized() const														{ return mNormalize; }

		/**
		 * Calculates normalization factor based on the given reference text.
		 * This is called automatically on initialization but can be changed at runtime.
		 * Caching this value ensures that at runtime the size of the text doesn't change when normalization is turned on.
		 * @param referenceText text used to calculate the normalization factor
		 * @return if the bounds updated correctly based on the reference text
		 */
		bool computeNormalizationFactor(const std::string& referenceText);

	protected:
		/**
		 * Draws the text to the currently active render target using the render service.
		 * The size of the text is directly related to the size of the font.
		 * This function is called by the render service when text is rendered with a user defined camera.
		 * In that case the viewMatrix is the world space camera location and the the projection matrix is defined by the camera type.
		 * This can be orthographic or perspective. It is recommended to only use a perspective camera when rendering text in 3D.
		 * The TransformComponent of the parent entity is used to place the text and is therefore required.
		 * @param viewMatrix the camera world space location
		 * @param projectionMatrix the camera projection matrix, orthographic or perspective
		 */
		virtual void onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) override;

	private:
		bool	mNormalize = true;						///< If the text as a mesh is normalized (-0.5,0.5)
		float	mNormalizationFactor = 1.0f;			///< Calculated normalization factor based on reference text
	};
}
