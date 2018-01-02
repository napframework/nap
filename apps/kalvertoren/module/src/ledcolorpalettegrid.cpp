#include "LedColorPaletteGrid.h"

#include <utility/fileutils.h>
#include <nbitmaputils.h>
#include <ntextureutils.h>
#include <fstream>

// nap::LedColorPaletteGrid run time class definition 
RTTI_BEGIN_CLASS(nap::LedColorPaletteGrid)
	RTTI_PROPERTY("GridPath",		&nap::LedColorPaletteGrid::mGridImagePath,		nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
	RTTI_PROPERTY("GridSize",		&nap::LedColorPaletteGrid::mGridSize,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("GridColorsPath", &nap::LedColorPaletteGrid::mGridLedColorsPath,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	LedColorPaletteGrid::~LedColorPaletteGrid()			{ }


	bool LedColorPaletteGrid::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mGridImagePath.empty(), "Image path not set for color palette %s", mID.c_str()))
			return false;

		// Load pixel data in to bitmap
		if (!mPixmap.initFromFile(mGridImagePath, errorState))
			return false;

		// Make sure the amount of channels is > 3
		if (!errorState.check(mPixmap.getBitmap().getNumberOfChannels() >= 3, "color palette map: %s does not have 3 channels", mGridImagePath.c_str()))
			return false;

		if (!initPaletteGrid(errorState))
			return false;

		// Get opengl settings from bitmap
		opengl::Texture2DSettings settings;
		if (!errorState.check(opengl::getSettingsFromBitmap(mPixmap.getBitmap(), false, settings, errorState), "Unable to determine texture settings from bitmap %s", mGridImagePath.c_str()))
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

	bool LedColorPaletteGrid::initPaletteGrid(utility::ErrorState& errorState)
	{
		float numColumns = (int)((float)mPixmap.mWidth / (float)mGridSize);
		float numRows = (int)((float)mPixmap.mHeight / (float)mGridSize);

		mPaletteGrid.resize(numRows);

		for (int row = 0; row < numRows; ++row)
		{
			for (int column = 0; column < numColumns; ++column)
			{
				// Note: we sample the the middle of the square since the squares aren't always exactly aligned on a grid due to bugs in Illustrator
				int x = column * mGridSize + (int)(0.5f * mGridSize);
				int y = row * mGridSize + (int)(0.5f * mGridSize);

				RGBAColor8 current_color = mPixmap.getColor<RGBAColor8>(x, mPixmap.mHeight - y - 1);
				if (current_color.getAlpha() == 0)
					break;

				PaletteColor color;
				color.mScreenColor = current_color.convert<RGBColor8>();
				mPaletteGrid[row].push_back(color);
			}
		}

		if (!initLedColorMapping(errorState))
			return false;

		return true;
	}

	bool LedColorPaletteGrid::initLedColorMapping(utility::ErrorState& errorState)
	{
		std::ifstream led_colors(mGridLedColorsPath);
		if (!errorState.check(led_colors.good(), "Failed to read LED colors from %s: file not found", mGridLedColorsPath.c_str()))
			return false;

		int row = 0;
		std::string currentLine;
		while (std::getline(led_colors, currentLine))
		{
			if (!errorState.check(row < mPaletteGrid.size(), "LED grid colors file %s has more rows than the grid image (%d in image, %d in text file)", mGridLedColorsPath.c_str(), mPaletteGrid.size(), row))
				return false;

			std::vector<std::string> columns;
			utility::splitString(currentLine, '/', columns);

			if (!errorState.check(columns.size() == mPaletteGrid[row].size(), "LED grid colors file %s has more columns on row %d than the grid image (%d in image, %d in text file)", mGridLedColorsPath.c_str(), row, mPaletteGrid[row].size(), columns.size()))
				return false;

			for (int column = 0; column < columns.size(); ++column)
			{
				const std::string& color = columns[column];

				int r, g, b, w;
				int numComponents = sscanf(color.c_str(), "%d %d %d %d", &r, &g, &b, &w);

				// Check that the color consists out of 4 components
				if (!errorState.check(numComponents == 4, "LED grid colors file %s has an invalid number of components for the color on row %d and column %d (expected 4, found %d)", mGridLedColorsPath.c_str(), row+1, column+1, numComponents))
					return false;

				// Check that the range of the R value is correct
				if (!errorState.check(r >= 0 && r <= 255, "LED grid colors file %s has an invalid R value for the color on row %d and column %d (expected value between 0 and 255, found %d)", mGridLedColorsPath.c_str(), row+1, column+1, r))
					return false;

				// Check that the range of the G value is correct
				if (!errorState.check(g >= 0 && g <= 255, "LED grid colors file %s has an invalid G value for the color on row %d and column %d (expected value between 0 and 255, found %d)", mGridLedColorsPath.c_str(), row+1, column+1, g))
					return false;

				// Check that the range of the B value is correct
				if (!errorState.check(b >= 0 && b <= 255, "LED grid colors file %s has an invalid B value for the color on row %d and column %d (expected value between 0 and 255, found %d)", mGridLedColorsPath.c_str(), row+1, column+1, b))
					return false;

				// Check that the range of the W value is correct
				if (!errorState.check(w >= 0 && w <= 255, "LED grid colors file %s has an invalid W value for the color on row %d and column %d (expected value between 0 and 255, found %d)", mGridLedColorsPath.c_str(), row+1, column+1, w))
					return false;

				mPaletteGrid[row][column].mLedColor = RGBAColor8(r, g, b, w);
			}

			row++;
		}

		if (!errorState.check(row == mPaletteGrid.size(), "LED grid colors file %s has fewer rows than the grid image (%d in image, %d in text file)", mGridLedColorsPath.c_str(), mPaletteGrid.size(), row))
			return false;

		return true;
	}
}