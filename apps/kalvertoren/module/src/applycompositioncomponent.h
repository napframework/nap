#pragma once

#include "applycolorcomponent.h"
#include "compositioncomponent.h"
#include "colorpalettecomponent.h"
#include "rendercompositioncomponent.h"

#include <component.h>
#include <componentptr.h>

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
		float										mIntensity = 1.0f;				///< property: intensity
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
		void showIndexColors(bool value)							{ mShowIndexColors = value; }

		/**
		 *	Sets the intensity of the colors applied to the mesh
		 */
		void setIntensity(float value)								{ mIntensity = value; }

		// pointer to the component that manages all the compositions
		COMPONENT_INSTANCE_POINTER(mCompositionRenderer, RenderCompositionComponent, ApplyCompositionComponent)

		// pointer to the component that manages the color palettes
		COMPONENT_INSTANCE_POINTER(mColorPaletteComponent, ColorPaletteComponent, ApplyCompositionComponent)

	private:
		nap::Pixmap mPixmap;
		bool mShowIndexColors = false;
		float mIntensity = 1.0f;
	};
}
