#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <basetexture2d.h>
#include <color.h>
#include <pixmap.h>

namespace nap
{
	/**
	 * Maps a bitmap image to a set of LED compatible colors
	 * The amount of colors extracted from the bitmap need to match
	 * the amount of declared LED colors
	 */
	class LedColorPalette : public BaseTexture2D
	{
		RTTI_ENABLE(BaseTexture2D)
	public:
		virtual ~LedColorPalette();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the total number of colors associated with this palette
		 */
		int getCount() const						{ return mPaletteColors.size(); }

		/**
		 * @return the color in the palette @index
		 */
		const RGBColor8& getPaletteColor(int index) const;

		/**
		 *	@return the led color in the palette @index
		 */
		const RGBAColor8& getLEDColor(int index) const;

		std::string				mImagePath;			///< Path to the palette image on disk
		std::vector<RGBAColor8> mLedColors;			///< All the LED colors associated with this palette (RGBA 8 bit)

	private:
		std::vector<RGBColor8>	mPaletteColors;		///< All the colors extracted from the palette (RGB 8 bit)
		nap::Pixmap				mPixmap;			///< Bitmap associated with this led color palette

		void findPaletteColors();					///< Retrieves all the index colors from the map
	};
}
