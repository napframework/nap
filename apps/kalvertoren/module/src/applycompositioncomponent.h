#pragma once

#include "applycolorcomponent.h"
#include "compositioncomponent.h"
#include "colorpalettecomponent.h"

#include <component.h>
#include <componentptr.h>

namespace nap
{
	class ApplyCompositionComponentInstance;

	/**
	 *	applycompositioncomponent
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

		ComponentPtr<CompositionComponent>  mCompositionComponent;			///< property: link to the composition component
		ComponentPtr<ColorPaletteComponent> mColorPaletteComponent;			///< property: link to the color palette component
	};


	/**
	 * applycompositioncomponentInstance	
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
		 *	Applies bounding box colors to the mesh
		 */
		virtual void applyColor(double deltaTime) override;

		// pointer to the component that manages all the compositions
		COMPONENT_INSTANCE_POINTER(mCompositionComponent, CompositionComponent, ApplyCompositionComponent)

		// pointer to the component that manages the color palettes
		COMPONENT_INSTANCE_POINTER(mColorPaletteComponent, ColorPaletteComponent, ApplyCompositionComponent)

	private:
		nap::Pixmap mPixmap;
	};
}
