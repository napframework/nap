#include "ledcolorpalettegrid.h"

#include <utility/fileutils.h>
#include <nbitmaputils.h>
#include <ntextureutils.h>
#include <fstream>

RTTI_BEGIN_CLASS(nap::WeekColors)
	RTTI_PROPERTY("Palette",		&nap::WeekColors::mPalette,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Variations",		&nap::WeekColors::mVariations,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::LedColorPaletteGrid run time class definition 
RTTI_BEGIN_CLASS(nap::LedColorPaletteGrid)
	RTTI_PROPERTY("GridPath",		&nap::LedColorPaletteGrid::mGridImagePath,		nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
	RTTI_PROPERTY("GridSize",		&nap::LedColorPaletteGrid::mGridSize,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("GridColorsPath", &nap::LedColorPaletteGrid::mGridLedColorsPath,	nap::rtti::EPropertyMetaData::Required | nap::rtti::EPropertyMetaData::FileLink)
	RTTI_PROPERTY("WeekColors",		&nap::LedColorPaletteGrid::mWeekColors,			nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool WeekColors::init(utility::ErrorState& errorState)
	{
		std::vector<std::string> grid_indices;
		utility::splitString(mPalette, ' ', grid_indices);

		// Convert palette strings to indices. Strings are in format '[A-Z][###]'. The letter denotes the column, the number the row.
		// Note that the rows are 1-based.
		for (int index = 0; index < grid_indices.size(); ++index)
		{
			const std::string& grid_index = grid_indices[index];

			char column_char;
			int row_index;
			int numComponents = sscanf(grid_index.c_str(), "%c%d", &column_char, &row_index);

			if (!errorState.check(numComponents == 2, "Encountered invalid format '%s' for color pattern at index %d in week %s", grid_index.c_str(), index, mID.c_str()))
				return false;

			int column_index = column_char - 'A';
			
			// Subtract 1 from row index since they're 1-based.
			mPaletteColors.push_back({ row_index-1, column_index });
		}

		// Verify that all variations are valid. They must have the same size as the palette and each element of a variation must be in range
		for (int index = 0; index < mVariations.size(); ++index)
		{
			const std::vector<int>& variation = mVariations[index];
			if (!errorState.check(variation.size() == mPaletteColors.size(), "Palette variation at index %d in week %s has a different number of components than the palette (%d in variation, %d in palette)", index, mID.c_str(), variation.size(), mPaletteColors.size()))
				return false;
			
			// Verify variation elements are in range
			for (int variation_index = 0; variation_index < variation.size(); ++variation_index)
			{
				int palette_index = variation[variation_index];
				if (!errorState.check(palette_index >= 0 && palette_index < mPaletteColors.size(), "Palette variation at index %d in week %s has an invalid palette index '%d' at element %d (must be between 0 and %d)", index, mID.c_str(), palette_index, variation_index, mPaletteColors.size()))
					return false;
			}
		}

		return true;
	}


	std::vector<WeekColors::GridColorIndex> WeekColors::getColors(int variationIndex) const
	{
		assert(variationIndex == -1 || (variationIndex > 0 && variationIndex < mVariations.size()));

		// Return base palette if -1 is specified
		if (variationIndex == -1)
			return mPaletteColors;

		// Otherwise shuffle the base palette
		std::vector<GridColorIndex> result;
		for (int palette_index : mVariations[variationIndex])
			result.push_back(mPaletteColors[palette_index]);

		return result;
	}


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

		if (!checkWeekColors(errorState))
			return false;

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


	bool LedColorPaletteGrid::checkWeekColors(utility::ErrorState& errorState)
	{
		// Verify all weeks refer to colors in the grid that actually exist
		for (auto& weekColors : mWeekColors)
		{
			std::vector<WeekColors::GridColorIndex> colors = weekColors->getColors(-1);
			for (const WeekColors::GridColorIndex& color_index : colors)
			{
				// Verify the cell is within range
				bool valid_cell = color_index.mRow >= 0 && color_index.mRow < mPaletteGrid.size() && color_index.mColumn >= 0 && color_index.mColumn < mPaletteGrid[color_index.mRow].size();
				if (!errorState.check(valid_cell, "Encountered week %s which has a pattern (%s) that references a cell in the grid (%c%d) that does not exist.", weekColors->mID.c_str(), weekColors->mPalette.c_str(), ('A'+color_index.mColumn), color_index.mRow+1))
					return false;
			}
		}
		return true;
	}


	int LedColorPaletteGrid::getWeekVariationCount(int weekNumber) const
	{
		assert(weekNumber < mWeekColors.size());
		return mWeekColors[weekNumber]->getVariationCount();
	}


	std::vector<LedColorPaletteGrid::PaletteColor> LedColorPaletteGrid::getPalette(int weekNumber, int variationIndex) const
	{
		assert(weekNumber < mWeekColors.size());
		assert(variationIndex == -1 || variationIndex < mWeekColors[weekNumber]->getVariationCount());

		std::vector<WeekColors::GridColorIndex> color_indices = mWeekColors[weekNumber]->getColors(variationIndex);
		
		std::vector<PaletteColor> result;
		for (const WeekColors::GridColorIndex& color_index : color_indices)
			result.push_back(mPaletteGrid[color_index.mRow][color_index.mColumn]);

		return result;
	}
}