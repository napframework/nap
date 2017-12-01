#include "colorpalettecomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>

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
		auto it = mIndexToPaletteMap.lower_bound(indexColor);
		assert(it != mIndexToPaletteMap.end());
		return it->second;
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