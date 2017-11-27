#include "ledcolorcontainer.h"

// nap::ledcolorcontainer run time class definition 
RTTI_BEGIN_CLASS(nap::LedColorContainer)
	RTTI_PROPERTY("IndexMap", &nap::LedColorContainer::mIndexMap,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Palettes", &nap::LedColorContainer::mColorPalettes,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	LedColorContainer::~LedColorContainer()			{ }


	bool LedColorContainer::init(utility::ErrorState& errorState)
	{
		int index_color_count = mIndexMap->getCount();
		for (auto& palette : mColorPalettes)
		{
			if (!errorState.check(palette->getCount() <= index_color_count, "Palette: %s has more colors than Index map: %s", palette->mImagePath.c_str(), mIndexMap->mImagePath.c_str()))
				return false;
		}
		return true;
	}
}
