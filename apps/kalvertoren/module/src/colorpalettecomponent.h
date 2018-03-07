#pragma once

#include "indexmap.h"
#include "ledcolorpalettegrid.h"
#include "imagefromfile.h"

#include <component.h>
#include <rtti/objectptr.h>

namespace nap
{
	class ColorPaletteComponentInstance;


   /**
	* Determines how the color palette component cycles through the available compositions
	*/
	enum class ColorPaletteCycleMode : int
	{
		Off			= 0,			///< Palettes do not cycle automatically
		Random		= 1,			///< A new palette is chosen when the active one finishes
		Sequence	= 2				///< Plays through all the compositions one by one
	};


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

		rtti::ObjectPtr<IndexMap>				mIndexMap;											///< Property: The index map to use
		rtti::ObjectPtr<LedColorPaletteGrid>	mPaletteGrid;										///< Property: The palette grid to use, containing palettes for each weak
		rtti::ObjectPtr<ImageFromFile>			mDebugImage;										///< Property: Debug image used to display the currently selected palette
		int								mIndex = 0;											///< Property: Current palette selection
		float							mCycleSpeed = 1.0f;									///< Property: Time it takes to jump to a new color palette
		ColorPaletteCycleMode			mVariationCycleMode = ColorPaletteCycleMode::Off;	///< Property: Default cycle mode
	};


	/**
	 * colorpalettecomponentInstance	
	 */
	class NAPAPI ColorPaletteComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		enum class EStatus : int
		{
			Active = 0,
			Completed
		};

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
		 *	@return whether the weeknumber of the current date should be used to select a palette
		 */
		bool getLockWeek() const { return mLockWeek; }

		/**
		 *	Set whether the weeknumber of the current date should be used to select a palette
		 */
		void setLockWeek(bool inLock) { mLockWeek = inLock; }

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
		 * Sets if we want to cycle through colors
		 * @param cycle if we want to cycle through the colors or not
		 */
		void setCycleMode(ColorPaletteCycleMode mode)								{ mVariationCycleMode = mode; }

		/**
		 * @return the current cycle mode
		 */
		ColorPaletteCycleMode getCycleMode() const									{ return mVariationCycleMode; }

		/**
		 * Sets the cycle speed in seconds
		 * @param speed cycle speed in seconds
		 */
		void setCycleSpeed(float speed)												{ mCycleSpeed = speed; }

		/**
		 * @return progress in 0-1 range
		 */
		float getProgress() const;

		/**
		 *	@return current color cycle status
		 */
		EStatus getStatus() const;

	private:
		/**
		 * Builds a map that binds the index colors to the currently selected palette colors
		 * Note that when the index color count > palette color count the first palette color is used
		 */
		void updateSelectedPalette();

		rtti::ObjectPtr<IndexMap>				mIndexMap;									///< The index map to use
		rtti::ObjectPtr<LedColorPaletteGrid>	mPaletteGrid;								///< The palette grid to use, containing palettes for each weak
		rtti::ObjectPtr<ImageFromFile>			mDebugImage;								///< Debug image used to display the currently selected palette

		// Map that binds index colors to current color palette colors
		std::map<IndexMap::IndexColor, LedColorPaletteGrid::PaletteColor> mIndexToPaletteMap;

		// Cycle Speed
		float mCycleSpeed = 1.0f;

		// Current time
		double mTime = 0.0;

		// Use week number of current date
		bool mLockWeek = true;

		// Current week
		int mCurrentWeek = -1;

		// Current Selection
		int mCurrentVariationIndex = 0;

		// Cycle Mode
		ColorPaletteCycleMode mVariationCycleMode = ColorPaletteCycleMode::Off;
	};
}
