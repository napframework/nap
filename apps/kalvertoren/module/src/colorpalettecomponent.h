#pragma once

#include "ledcolorcontainer.h"

#include <component.h>
#include <nap/objectptr.h>

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

		ObjectPtr<LedColorContainer>	mColors = nullptr;							///< Property: Link to all the available colors and the index map
		int								mIndex = 0;									///< Property: Current palette selection
		float							mCycleSpeed = 1.0f;							///< Property: Time it takes to jump to a new color palette
		ColorPaletteCycleMode			mCycleMode = ColorPaletteCycleMode::Off;	///< Property: Default cycle mode
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
		 *	@return the total number of available color palettes
		 */
		int getCount() const;

		/**
		 *	Selects a new color palette to be used
		 */
		void select(int index);

		/**
		 *	@return the currently selected color palette
		 */
		LedColorPalette& getSelection()												{ return *mSelection; }

		/**
		 *	@return the currently selected color palette
		 */
		const LedColorPalette& getSelection() const									{ return *mSelection; }

		/**
		 *	@return the index map
		 */
		const IndexMap& getIndexMap() const;

		/**
		 *	@return the index map
		 */
		IndexMap& getIndexMap();

		/**
		 * @param indexColor the color to get the associated palette color for
		 * @return the palette color associated with a certain index map color
		 */
		const RGBColor8& getPaletteColor(const IndexMap::IndexColor& indexColor) const;

		/**
		 *	@return the led color associated with @paletteColor
		 */
		const RGBAColor8& getLedColor(const RGBColor8& paletteColor) const;

		/**
		 * Sets if we want to cycle through colors
		 * @param cycle if we want to cycle through the colors or not
		 */
		void setCycleMode(ColorPaletteCycleMode mode)								{ mCycleMode = mode; }

		/**
		 * @return the current cycle mode
		 */
		ColorPaletteCycleMode getCycleMode() const									{ return mCycleMode; }

		/**
		 * Sets the cycle speed in seconds
		 * @param speed cycle speed in seconds
		 */
		void setCycleSpeed(float speed)												{ mCycleSpeed = speed; }

	private:
		/**
		 * Builds a map that binds the index colors to the currently selected palette colors
		 * Note that when the index color count > palette color count the first palette color is used
		 */
		void buildMap();

		// All color palettes including the index map
		LedColorContainer* mContainer = nullptr;

		// Currently selected color palette
		LedColorPalette* mSelection = nullptr;

		// Map that binds index colors to current color palette colors
		std::map<IndexMap::IndexColor, RGBColor8> mIndexToPaletteMap;

		// Cycle Speed
		float mCycleSpeed = 1.0f;

		// Current time
		double mTime = 0.0;

		// Current Selection
		int mCurrentIndex = -1;

		// Cycle Mode
		ColorPaletteCycleMode mCycleMode = ColorPaletteCycleMode::Off;
	};
}
