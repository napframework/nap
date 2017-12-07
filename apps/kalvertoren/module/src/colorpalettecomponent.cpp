#include "colorpalettecomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <nap/logger.h>

// nap::colorpalettecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ColorPaletteComponent)
	RTTI_PROPERTY("Colors", &nap::ColorPaletteComponent::mColors, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Index", &nap::ColorPaletteComponent::mIndex, nap::rtti::EPropertyMetaData::Default)
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
		// Copy pointer to container
		mContainer = getComponent<ColorPaletteComponent>()->mColors.get();
		if (!errorState.check(mContainer->getCount() > 0, "No color palettes specified: %s", mContainer->mID.c_str()))
			return false;

		// Select current palette based on loaded index
		select(getComponent<ColorPaletteComponent>()->mIndex);

		return true;
	}


	void ColorPaletteComponentInstance::update(double deltaTime)
	{

	}


	int ColorPaletteComponentInstance::getCount() const
	{
		return mContainer->getCount();
	}


	void ColorPaletteComponentInstance::select(int index)
	{
		int sidx = nap::math::clamp<int>(index, 0, mContainer->getCount() - 1);
		mSelection = mContainer->mColorPalettes[sidx].get();
		buildMap();
	}


	nap::IndexMap& ColorPaletteComponentInstance::getIndexMap()
	{
		return *(mContainer->mIndexMap.get());
	}


	const nap::IndexMap& ColorPaletteComponentInstance::getIndexMap() const
	{
		return *(mContainer->mIndexMap.get());
	}


	const nap::RGBColor8& ColorPaletteComponentInstance::getPaletteColor(const IndexMap::IndexColor& indexColor) const
	{
		// Perfom a direct lookup (fastest)
		auto it = mIndexToPaletteMap.find(indexColor);
		if (it != mIndexToPaletteMap.end())
			return it->second;

		/*
		float dist = nap::math::max<float>();
		const nap::RGBColor8* found_color = nullptr;

		for (auto& color : mIndexToPaletteMap)
		{
			float cdist = color.second.getDistance(indexColor);
			if (cdist < dist)
			{
				dist = cdist;
				found_color = &(color.second);
			}
		}
		return *found_color;
		*/

		// If the color can't be found we need to look for the upper and lower bounds of the wrapping colors
		auto upper_it = mIndexToPaletteMap.lower_bound(indexColor);
		assert(upper_it != mIndexToPaletteMap.end());

		// Get lower bound
		auto lower_it = upper_it == mIndexToPaletteMap.begin() ? mIndexToPaletteMap.end() : upper_it;
		--lower_it;

		// Get distance from index to both bounds
		float d_lower = lower_it->second.getDistance(indexColor);
		float d_highr = upper_it->second.getDistance(indexColor);

		// Select closest
		return d_lower < d_highr ? lower_it->second : upper_it->second;
	}


	const nap::RGBAColor8& ColorPaletteComponentInstance::getLedColor(const RGBColor8& paletteColor) const
	{
		return mSelection->getLEDColor(paletteColor);
	}


	void ColorPaletteComponentInstance::buildMap()
	{
		// Build a map that maps the index colors to palette colors
		mIndexToPaletteMap.clear();

		int count = 0;
		for (const auto& index_color : getIndexMap().getColors())
		{
			RGBColor8 palette_color = count < mSelection->getCount() ? mSelection->getPaletteColor(count) : mSelection->getPaletteColor(0);
			mIndexToPaletteMap.emplace(std::make_pair(index_color, palette_color));
			count++;
		}
	}
}