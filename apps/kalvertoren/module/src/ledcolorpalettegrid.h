#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <texture2d.h>
#include <color.h>
#include <bitmap.h>
#include <unordered_map>
#include "nap/objectptr.h"

namespace nap
{
	class LedColorPaletteGrid;

	/**
	 *	Defines the variations for every week
	 */
	class NAPAPI WeekVariations : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		std::vector<std::vector<int>>	mVariations;		///< The variations for this week. Each element is an array of indices into the palette for this week.
		
		/**
		 * @return total amount of available variations
		 */
		int getCount() const			{ return mVariations.size(); }
	};


	/**
	 * Defines the palette colors for a particular week
	 */
	class NAPAPI WeekColors : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		// Helper struct to represent an index into the color palette grid
		struct GridColorIndex
		{
			int mRow;		///< The row in the grid
			int mColumn;	///< The column in the grid
		};

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Get the number of variations in this palette
		 */
		int getVariationCount() const						{	return mVariations->mVariations.size(); }
	
		/**
		 * Get the palette colors for the specified variation
		 * @param variationIndex The index of the variation to get the colors for. If this is -1, the base palette colors will be returned. Otherwise it must be between 0 and getVariationCount()
		 */
		std::vector<GridColorIndex> getColors(int variationIndex) const;

	public:
		std::string						mPalette;			///< The palette pattern. Each element (separated by a space) denotes an index in the palette grid
		ObjectPtr<WeekVariations>		mVariations;		///< The variations based on the pattern

	private:
		std::vector<GridColorIndex>	mPaletteColors;
	};

	/**
	 * Represents a color grid palette. Extracts the colors from and image and the LED colors from the corresponding txt file.
	 * Allows the client to get the palette for a given week and variation
	 */
	class NAPAPI LedColorPaletteGrid : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		struct PaletteColor
		{
			RGBColor8	mScreenColor;	///< The color that should be used for rendering to the screen
			RGBAColor8	mLedColor;		///< The color that should be used for driving the LED 
		};

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;
		
		/**
		 * Get the number of weeks that are actually defined in the grid
		 */
		int getWeekCount() const { return mWeekColors.size(); }

		/**
		 * Get the number of variations for the specified week
		 */
		int getWeekVariationCount(int weekNumber) const;

		/**
		 * Get the palette for the given week and variation.
		 *
		 * @param weekNumber The week (0-51) to get the palette for
		 * @param variationIndex The variation in the week to get the palette for. Specify -1 to get the base palette. Otherwise it must be between 0 and getWeekVariationCount().
		 */
		std::vector<PaletteColor> getPalette(int weekNumber, int variationIndex) const;

	private:
		bool initPaletteGrid(utility::ErrorState& errorState);		///< Read the palette grid colors from the image
		bool initLedColorMapping(utility::ErrorState& errorState);	///< Read the led colors from the txt file
		bool checkWeekColors(utility::ErrorState& errorState);		///< Verify that the colors & mappings specified in the WeekColors array are correct

	public:
		std::string							mGridImagePath;			///< Path to the palette grid image on disk
		std::string							mGridLedColorsPath;		///< Path to the palette grid led colors text file on disk
		int									mGridSize;				///< Size of each square in the grid (in pixels)
		std::vector<ObjectPtr<WeekColors>>	mWeekColors;			///< The colors for each week

	private:
		using PaletteRow = std::vector<PaletteColor>;
		using PaletteGrid = std::vector<PaletteRow>;

		nap::Bitmap				mBitmap;						///< Bitmap associated with this led color palette
		PaletteGrid				mPaletteGrid;					///< All the colors extracted from the palette, divided into a grid (RGB 8 bit)
	};
}
