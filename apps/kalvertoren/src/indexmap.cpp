#include "indexmap.h"

// External Includes
#include <nbitmaputils.h>
#include <ntextureutils.h>
#include <utility/fileutils.h>
#include <texture2d.h>

// nap::indexmap run time class definition 
RTTI_BEGIN_CLASS(nap::IndexMap)
	RTTI_PROPERTY("Path", &nap::IndexMap::mImagePath, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ColorCount", &nap::IndexMap::mColorCount, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	IndexMap::~IndexMap()			{ }

	bool IndexMap::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mImagePath.empty(), "Image path not set for index map %s", mID.c_str()))
			return false;

		// Make sure it's a .bmp file
		if (!errorState.check(utility::getFileExtension(mImagePath) == "bmp", "Image is not a bitmap: %s", mImagePath.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!mPixmap.initFromFile(mImagePath, errorState))
			return false;

		// Make sure the amount of channels is > 3
		if (!errorState.check(mPixmap.getBitmap().getNumberOfChannels() >= 3, "Index map: %s does not have 3 channels", mImagePath.c_str()))
			return false;

		// Make sure it's a 24 bit map
		if (!errorState.check(mPixmap.mType == Pixmap::EDataType::BYTE, "Index map is not 8bit per channel", mImagePath.c_str()))
			return false;

		// Get all unique colors from the map
		findUniqueColors();

		// Make sure the amount of colors matches the amount of expected colors to find
		if (!errorState.check(mColorCount == mIndexColors.size(), "Expected to find %d colors, got %d instead: %s", mColorCount, mIndexColors.size(), mImagePath.size()))
			return false;

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
		getTexture().setData(mPixmap.getBitmap().getData());

		return true;
	}


	const nap::IndexMap::IndexColor& IndexMap::getColor(int id) const
	{
		assert(id >= 0 && id < mIndexColors.size());
		return mIndexColors[id];
	}


	int IndexMap::getCount() const
	{
		return static_cast<int>(mIndexColors.size());
	}


	void IndexMap::findUniqueColors()
	{
		RGBColorFloat floatcolor;
		RGBColor8 bytecolor(255, 128, 0);
		bytecolor.convert(floatcolor);

		std::unordered_set<IndexColor> unique_index_colors;

		// Check the amount of available colors
		for (int i = 0; i < mPixmap.mWidth; i++)
		{
			// Get color value at pixel and compare
			RGBColor8 current_color = mPixmap.getRGBColor<uint8>(i, 0);

			auto& it = unique_index_colors.find(current_color);
			if (it != unique_index_colors.end())
				continue;

			unique_index_colors.emplace(current_color);
			mIndexColors.emplace_back(current_color);
		}
	}

}