#pragma once

#include "indexmap.h"
#include "ledcolorpalettegrid.h"
#include "selectcolormethodcomponent.h"
#include "compositioncomponent.h"

#include <imagefromfile.h>
#include <component.h>
#include <nap/resourceptr.h>

namespace nap
{
	class ColorPaletteComponentInstance;

	/**
	 *	colorpalettecomponent
	 */
	class NAPAPI ColorPaletteComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ColorPaletteComponent, ColorPaletteComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<IndexMap>				mIndexMap;											///< Property: The index map to use
		ResourcePtr<LedColorPaletteGrid>	mPaletteGrid;										///< Property: The palette grid to use, containing palettes for each weak
		ResourcePtr<ImageFromFile>			mDebugImage;										///< Property: Debug image used to display the currently selected palette
		int									mIndex = 0;											///< Property: Current palette selection
		ComponentPtr<CompositionComponent>	mCompositionComponent;								///< Property: Link to the composition component
		bool								mLinkToComposition = true;							///< Property: If we want to listen to composition changes;
	};


	/**
	 * colorpalettecomponentInstance	
	 */
	class NAPAPI ColorPaletteComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:

		ColorPaletteComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize colorpalettecomponentInstance based on the colorpalettecomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the colorpalettecomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update colorpalettecomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	@return the total number of available variations for the currently selected week
		 */
		int getVariationCount() const;

		/**
		 * Selects a week to be used to get the palette from
		 */
		void selectWeek(int index);

		/**
		 * Get the currently selected week
		 */
		int getSelectedWeek() const { return mCurrentWeek; }

		/**
		 *	Selects a new variation within the week's color palette to be used
		 */
		void selectVariation(int index);

		/**
		 *	@return the current variation index
		 */
		int getVariation() const { return mCurrentVariationIndex; }

		/**
		 *	@return the index map
		 */
		const IndexMap& getIndexMap() const;

		/**
		 *	@return the index map
		 */
		IndexMap& getIndexMap();

		/**
		 * @return The debug palette image
		 */
		ImageFromFile& getDebugPaletteImage();

		/**
		 * @param indexColor the color to get the associated palette color for
		 * @return the palette color associated with a certain index map color
		 */
		LedColorPaletteGrid::PaletteColor getPaletteColor(const IndexMap::IndexColor& indexColor) const;

		/**
		 * Resolved link to runtime composition component
		 */
		ComponentInstancePtr<CompositionComponent> mCompositionComp =				{ this, &ColorPaletteComponent::mCompositionComponent };

		/**
		 * Links the change of color to completion of composition
		 * @param value if this object listens to composition changes or not
		 */
		bool mLinked = false;															///< If we're currently switching based on component changes

		/**
		 * If the week is locked
		 */
		bool mLockWeek = true;

		/**
		 *	If the week selection is locked
		 */
		bool isLocked() const { return mLockWeek; }

	private:
		/**
		 * Builds a map that binds the index colors to the currently selected palette colors
		 * Note that when the index color count > palette color count the first palette color is used
		 */
		void updateSelectedPalette();

		ResourcePtr<IndexMap>				mIndexMap;									///< The index map to use
		ResourcePtr<LedColorPaletteGrid>	mPaletteGrid;								///< The palette grid to use, containing palettes for each weak
		ResourcePtr<ImageFromFile>			mDebugImage;								///< Debug image used to display the currently selected palette


		// Map that binds index colors to current color palette colors
		std::map<IndexMap::IndexColor, LedColorPaletteGrid::PaletteColor> mIndexToPaletteMap;

		// Cycle Speed
		float mCycleSpeed = 1.0f;

		// Current week
		int mCurrentWeek = -1;

		// Current Selection
		int mCurrentVariationIndex = 0;

		void onCompositionChanged(const CompositionComponentInstance& composition);
		NSLOT(mCompChangedSlot, const CompositionComponentInstance&, onCompositionChanged);
	};
}
