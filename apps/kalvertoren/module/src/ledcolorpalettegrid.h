#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <basetexture2d.h>
#include <color.h>
#include <pixmap.h>
#include <unordered_map>

namespace nap
{
	/**
	 * Maps a bitmap image to a set of LED compatible colors
	 * The amount of colors extracted from the bitmap need to match
	 * the amount of declared LED colors
	 */
	class NAPAPI LedColorPaletteGrid : public BaseTexture2D
	{
		RTTI_ENABLE(BaseTexture2D)
	public:
		virtual ~LedColorPaletteGrid();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		std::string				mGridImagePath;					///< Path to the palette grid image on disk
		std::string				mGridLedColorsPath;				///< Path to the palette grid led colors text file on disk
		int						mGridSize;						///< Size of each square in the grid (in pixels)

	private:
		bool initPaletteGrid(utility::ErrorState& errorState);		///< Read the palette grid colors from the image
		bool initLedColorMapping(utility::ErrorState& errorState);	///< Read the led colors from the txt file

	private:
		struct PaletteColor
		{
			RGBColor8	mScreenColor;	///< The color that should be used for rendering to the screen
			RGBAColor8	mLedColor;		///< The color that should be used for driving the LED 
		};

		using PaletteRow = std::vector<PaletteColor>;
		using PaletteGrid = std::vector<PaletteRow>;

		nap::Pixmap				mPixmap;						///< Bitmap associated with this led color palette
		PaletteGrid				mPaletteGrid;					///< All the colors extracted from the palette, divided into a grid (RGB 8 bit)
	};
}
