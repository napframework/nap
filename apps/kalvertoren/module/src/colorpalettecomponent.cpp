#include "colorpalettecomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>
#include <utility/datetimeutils.h>

RTTI_BEGIN_ENUM(nap::ColorPaletteCycleMode)
	RTTI_ENUM_VALUE(nap::ColorPaletteCycleMode::Off, "Off"),
	RTTI_ENUM_VALUE(nap::ColorPaletteCycleMode::Random, "Random"),
	RTTI_ENUM_VALUE(nap::ColorPaletteCycleMode::Sequence, "List")
RTTI_END_ENUM

// nap::colorpalettecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ColorPaletteComponent)
	RTTI_PROPERTY("IndexMap",			&nap::ColorPaletteComponent::mIndexMap,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PaletteGrid",		&nap::ColorPaletteComponent::mPaletteGrid,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("DebugImage",			&nap::ColorPaletteComponent::mDebugImage,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index",				&nap::ColorPaletteComponent::mIndex,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("VariationCycleMode",	&nap::ColorPaletteComponent::mVariationCycleMode,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Speed",				&nap::ColorPaletteComponent::mCycleSpeed,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::colorpalettecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ColorPaletteComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ColorPaletteComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ColorPaletteComponentInstance::init(utility::ErrorState& errorState)
	{
		ColorPaletteComponent* color_palette_component = getComponent<ColorPaletteComponent>();
		mIndexMap = color_palette_component->mIndexMap;
		mPaletteGrid = color_palette_component->mPaletteGrid;
		mDebugImage = color_palette_component->mDebugImage;

		mCycleSpeed = getComponent<ColorPaletteComponent>()->mCycleSpeed;
		mVariationCycleMode = getComponent<ColorPaletteComponent>()->mVariationCycleMode;

		if (!errorState.check(mIndexMap->getBitmap().getWidth() == mDebugImage->getBitmap().getWidth() && mIndexMap->getBitmap().getHeight() == mDebugImage->getBitmap().getHeight(),
			"The dimensions of the IndexMap (%s) and DebugImage (%s) must match", mIndexMap->mID.c_str(), mDebugImage->mID.c_str()))
		{
			return false;
		}

		if (!errorState.check(mDebugImage->getBitmap().mChannels == Bitmap::EChannels::BGRA, "DebugImage (%s) must be a 4-channel BGRA texture", mDebugImage->mID.c_str()))
			return false;

		// Select current week
		selectWeek(utility::getCurrentDateTime().getWeek()-1);

		return true;
	}


	void ColorPaletteComponentInstance::update(double deltaTime)
	{
		if (mLockWeek)
		{
			int newWeekNumber = utility::getCurrentDateTime().getWeek()-1;
			if (newWeekNumber != mCurrentWeek)
			{
				selectWeek(newWeekNumber);
			}
		}

		if (mTime >= mCycleSpeed)
		{
			// Switch based on action to take
			switch (mVariationCycleMode)
			{
				case ColorPaletteCycleMode::Off:
				{
					break;
				}
				case ColorPaletteCycleMode::Random:
				{
					int new_idx = mCurrentVariationIndex;
					if (getVariationCount() > 1)
					{
						while (new_idx == mCurrentVariationIndex)
						{
							new_idx = math::random<int>(0, getVariationCount() - 1);
						}
					}
					selectVariation(new_idx);
					break;
				}
				case ColorPaletteCycleMode::Sequence:
				{
					if (getVariationCount() > 0)
					{
						int new_index = (mCurrentVariationIndex + 1) % static_cast<int>(getVariationCount());
						selectVariation(new_index);
					}
					break;
				}
			}
		}
		mTime += deltaTime;
	}


	int ColorPaletteComponentInstance::getVariationCount() const
	{
		if (mCurrentWeek >= mPaletteGrid->getWeekCount())
			return 0;

		return mPaletteGrid->getWeekVariationCount(mCurrentWeek);
	}


	void ColorPaletteComponentInstance::selectWeek(int index)
	{
		int sidx = nap::math::clamp<int>(index, 0, mPaletteGrid->getWeekCount() - 1);
		mCurrentWeek = sidx;
		selectVariation(0);
	}


	void ColorPaletteComponentInstance::selectVariation(int index)
	{
		mCurrentVariationIndex = nap::math::clamp<int>(index, 0, getVariationCount() - 1);
		mTime = 0.0;
		updateSelectedPalette();
	}


	nap::IndexMap& ColorPaletteComponentInstance::getIndexMap()
	{
		return *mIndexMap;
	}


	const nap::IndexMap& ColorPaletteComponentInstance::getIndexMap() const
	{
		return *mIndexMap;
	}


	ImageFromFile& ColorPaletteComponentInstance::getDebugPaletteImage()
	{
		return *getComponent<ColorPaletteComponent>()->mDebugImage;
	}


	LedColorPaletteGrid::PaletteColor ColorPaletteComponentInstance::getPaletteColor(const IndexMap::IndexColor& indexColor) const
	{
		// Perform a direct lookup (fastest)
		auto it = mIndexToPaletteMap.find(indexColor);
		if (it != mIndexToPaletteMap.end())
			return it->second;

		// If the color can't be found we need to look for the upper and lower bounds of the wrapping colors
		auto upper_it = mIndexToPaletteMap.lower_bound(indexColor);
		assert(upper_it != mIndexToPaletteMap.end());

		// Get lower bound
		auto lower_it = upper_it == mIndexToPaletteMap.begin() ? mIndexToPaletteMap.end() : upper_it;
		--lower_it;

		// Get distance from index to both bounds
		float d_lower = lower_it->second.mScreenColor.getDistance(indexColor);
		float d_highr = upper_it->second.mScreenColor.getDistance(indexColor);

		// Select closest
		return d_lower < d_highr ? lower_it->second : upper_it->second;
	}


	float ColorPaletteComponentInstance::getProgress() const
	{
		return math::fit<float>(mTime, 0.0f, mCycleSpeed, 0.0f, 1.0f);
	}


	nap::ColorPaletteComponentInstance::EStatus ColorPaletteComponentInstance::getStatus() const
	{
		return mTime > mCycleSpeed ? EStatus::Completed : EStatus::Active;
	}


	void ColorPaletteComponentInstance::updateSelectedPalette()
	{
		// Build a map that maps the index colors to palette colors
		mIndexToPaletteMap.clear();

		std::vector<LedColorPaletteGrid::PaletteColor> palette_colors = mPaletteGrid->getPalette(mCurrentWeek, mCurrentVariationIndex);

		int count = 0;
		for (const auto& index_color : getIndexMap().getColors())
		{
			LedColorPaletteGrid::PaletteColor palette_color = count < palette_colors.size() ? palette_colors[count] : palette_colors.back();
			mIndexToPaletteMap.emplace(std::make_pair(index_color, palette_color));
			count++;
		}

		// Update the debug texture
		int indexColorCount = mIndexMap->getColors().size();
		int imageWidth = mDebugImage->getBitmap().getWidth();
		int imageHeight = mDebugImage->getBitmap().getHeight();
		int squareWidth = imageWidth / indexColorCount;

		for (int y = 0; y < imageHeight; ++y)
		{
			for (int x = 0; x < imageWidth; ++x)
			{
				int palette_color_index = (int)std::floor(x / (float)squareWidth);

				RGBAColor8 color(0,0,0,0);
				if (palette_color_index < palette_colors.size())
				{
					color.setRed(palette_colors[palette_color_index].mScreenColor.getRed());
					color.setGreen(palette_colors[palette_color_index].mScreenColor.getGreen());
					color.setBlue(palette_colors[palette_color_index].mScreenColor.getBlue());
					color.setAlpha(255);
				}

				mDebugImage->getBitmap().setPixelColor<RGBAColor8>(x, y, color);
			}
		}

		mDebugImage->update(mDebugImage->getBitmap());
	}
}