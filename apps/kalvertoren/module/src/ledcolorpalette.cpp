#include "ledcolorpalette.h"

#include <utility/fileutils.h>
#include <nbitmaputils.h>
#include <ntextureutils.h>
#include <pixmap.h>

// nap::ledcolorpalette run time class definition 
RTTI_BEGIN_CLASS(nap::LedColorPalette)
	RTTI_PROPERTY("Path", &nap::LedColorPalette::mImagePath, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LedColors", &nap::LedColorPalette::mLedColors, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	LedColorPalette::~LedColorPalette()			{ }


	bool LedColorPalette::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Image path not set for color palette %s", mID.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!mPixmap.initFromFile(mImagePath, errorState))
			return false;

		// Make sure the amount of channels is > 3
		if (!errorState.check(mPixmap.getBitmap().getNumberOfChannels() >= 3, "color palette map: %s does not have 3 channels", mImagePath.c_str()))
			return false;

		// Now find all the available palette colors
		findPaletteColors();

		// Ensure that the amount of extracted colors matches the available laser colors
		if (!errorState.check(mLedColors.size() == mPaletteColors.size(), "mismatch detected in led and palette colors: %s, found led colors: %d, found palette colors: %d", mID.c_str(), mLedColors.size(), mPaletteColors.size()))
			return false;

		// Create the palette to led mapping
		int i = 0;
		for (auto& color : mPaletteColors)
		{
			mColorMap.emplace(std::make_pair(color, mLedColors[i]));
			i++;
		}

		// Get opengl settings from bitmap
		opengl::Texture2DSettings settings;
		if (!errorState.check(opengl::getSettingsFromBitmap(mPixmap.getBitmap(), false, settings, errorState), "Unable to determine texture settings from bitmap %s", mImagePath.c_str()))
			return false;

		// Force parameters
		mParameters.mMaxFilter = EFilterMode::Nearest;
		mParameters.mMinFilter = EFilterMode::Nearest;

		// Initialize texture from bitmap
		BaseTexture2D::init(settings);

		// Set data from bitmap
		setData(mPixmap);

		return true;
	}


	const RGBColor8& LedColorPalette::getPaletteColor(int index) const
	{
		assert(index >= 0 && index < mPaletteColors.size());
		return mPaletteColors[index];
	}


	const nap::RGBAColor8& LedColorPalette::getLEDColor(int index) const
	{
		assert(index >= 0 && index < mLedColors.size());
		return mLedColors[index];
	}

	
	const nap::RGBAColor8& LedColorPalette::getLEDColor(const RGBColor8& paletteColor) const
	{
		auto it = mColorMap.find(paletteColor);
		assert(it != mColorMap.end());
		return it->second;
	}


	void LedColorPalette::findPaletteColors()
	{
		RGBColor8 current_pixel;
		std::unique_ptr<BaseColor> source_pixel = mPixmap.makePixel();

		for (int i = 0; i < mPixmap.mWidth; i++)
		{
			mPixmap.getPixel(i, 0, *source_pixel);
			source_pixel->convert(current_pixel);

			if (mPaletteColors.empty() || mPaletteColors.back() != current_pixel)
			{
				mPaletteColors.emplace_back(current_pixel);
			}
		}
	}
}