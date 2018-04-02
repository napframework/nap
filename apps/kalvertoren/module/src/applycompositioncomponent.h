#pragma once

#include "applycolorcomponent.h"
#include "compositioncomponent.h"
#include "colorpalettecomponent.h"
#include "rendercompositioncomponent.h"

#include <component.h>
#include <componentptr.h>
#include <triangleiterator.h>

namespace nap
{
	class ApplyCompositionComponentInstance;

	/**
	 *	Applies the rendered compoisition to the mesh
	 */
	class NAPAPI ApplyCompositionComponent : public ApplyColorComponent
	{
		RTTI_ENABLE(ApplyColorComponent)
		DECLARE_COMPONENT(ApplyCompositionComponent, ApplyCompositionComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<RenderCompositionComponent>	mCompositionRenderer;			///< property: link to the composition component
		ComponentPtr<ColorPaletteComponent>			mColorPaletteComponent;			///< property: link to the color palette component
		bool										mShowIndexColors = false;		///< property: if the index colors should be shown
		bool										mBlendToWhite = false;			///< property: if the colors should blend to white
		int											mBlendAxis = 1;					///< property: axis used to blend over (0 = x, 1 = y, 2 = z)
		glm::vec2									mBlendRange = { 0.4, 0.6 };		///< property: where to blend to white
		float										mBlendPower = 1.0f;				///< property: blend power
	};


	/**
	 * Applies a rendered composition to the assigned mesh
	 */
	class NAPAPI ApplyCompositionComponentInstance : public ApplyColorComponentInstance
	{
		RTTI_ENABLE(ApplyColorComponentInstance)
	public:
		ApplyCompositionComponentInstance(EntityInstance& entity, Component& resource) :
			ApplyColorComponentInstance(entity, resource)										{ }

		/**
		 * Initialize applycompositioncomponentInstance based on the applycompositioncomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the applycompositioncomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Applies bounding box colors to the mesh
		 */
		virtual void applyColor(double deltaTime) override;

		/**
		 * Debug helper method that allows the display of the index colors instead of the composition colors
		 * Handy when trying to figure out if the index colors look correct on the mesh
		 */
		void showIndexColors(bool value)												{ mShowIndexColors = value; }

		/**
		 * If when applied the colors should blend to white
		 */
		void blendToWhite(bool value)													{ mBlendToWhite = value; }

		/**
		 * Applies the blend axis (0 = x, 1 = y, 2 = z)
		 */
		void setBlendAxis(int axis);

		/**
		 *	Sets the blend range
		 */
		void setBlendRange(const glm::vec2& range);

		// pointer to the component that manages all the compositions
		ComponentInstancePtr<RenderCompositionComponent> mCompositionRenderer	=		{ this, &ApplyCompositionComponent::mCompositionRenderer };

		// pointer to the component that manages all color palettes
		ComponentInstancePtr<ColorPaletteComponent> mColorPaletteComponent		=		{ this, &ApplyCompositionComponent::mColorPaletteComponent };

	private:
		bool		mShowIndexColors = false;
		bool		mBlendToWhite = false;
		int			mBlendAxis = 1;
		glm::vec2	mBlendRange = { 0.4f, 0.6f };
		float		mBlendPower = 1.0f;

		// White LED Color
		static const RGBAColorFloat mWhiteLedColor;

		// White RGB Color
		static const RGBColorFloat mWhiteRGBColor;
	};
}
