#include "ledcolorpalette.h"

#include <utility/fileutils.h>
#include <nbitmaputils.h>
#include <ntextureutils.h>

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

		// Make sure it's a .bmp file
		if (!errorState.check(utility::getFileExtension(mImagePath) == "bmp", "Image is not a bitmap: %s", mImagePath.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!errorState.check(opengl::loadBitmap(mBitmap, mImagePath, errorState), "Failed to load color palette %s; invalid bitmap", mImagePath.c_str()))
			return false;

		// Make sure the amount of channels is > 3
		if (!errorState.check(mBitmap.getNumberOfChannels() >= 3, "color palette map: %s does not have 3 channels", mImagePath.c_str()))
			return false;

		// Make sure it's a 24 bit map
		if (!errorState.check(mBitmap.getDataType() == opengl::BitmapDataType::BYTE, "color palette is not 8bit per channel", mImagePath.c_str()))
			return false;

		// Now find all the available palette colors
		findPaletteColors();

		// Ensure that the amount of extracted colors matches the available laser colors
		if (!errorState.check(mLedColors.size() == mPaletteColors.size(), "mismatch detected in led and palette colors, found led colors: %d, found palette colors: %d", mLedColors.size(), mPaletteColors.size()))
			return false;

		// Get opengl settings from bitmap
		opengl::Texture2DSettings settings;
		if (!errorState.check(opengl::getSettingsFromBitmap(mBitmap, false, settings, errorState), "Unable to determine texture settings from bitmap %s", mImagePath.c_str()))
			return false;

		// Force parameters
		mParameters.mMaxFilter = EFilterMode::Nearest;
		mParameters.mMinFilter = EFilterMode::Nearest;

		// Initialize texture from bitmap
		BaseTexture2D::init(settings);

		// Set data from bitmap
		getTexture().setData(mBitmap.getData());

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

	
	void LedColorPalette::findPaletteColors()
	{
		std::unordered_set<RGBColor8> unique_index_colors;

		// Check the amount of available colors
		for (int i = 0; i < mBitmap.getWidth(); i++)
		{
			uint8* pix_color = mBitmap.getPixel<uint8>(i, 0);
			RGBColor8 current_color;
			switch (mBitmap.getColorType())
			{
			case opengl::BitmapColorType::BGR:
				current_color.setValue(EColorChannel::Red, *(pix_color + 2));
				current_color.setValue(EColorChannel::Green, *(pix_color + 1));
				current_color.setValue(EColorChannel::Blue, *(pix_color + 0));
				break;
			case opengl::BitmapColorType::RGB:
				current_color.setValue(EColorChannel::Red, *(pix_color + 0));
				current_color.setValue(EColorChannel::Green, *(pix_color + 1));
				current_color.setValue(EColorChannel::Blue, *(pix_color + 2));
				break;
			default:
				assert(false);
				break;
			}

			auto& it = unique_index_colors.find(current_color);
			if (it == unique_index_colors.end())
			{
				unique_index_colors.emplace(current_color);
				mPaletteColors.emplace_back(current_color);
			}
		}
	}

}