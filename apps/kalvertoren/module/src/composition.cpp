#include "composition.h"

#include <mathutils.h>

// nap::composition run time class definition 
RTTI_BEGIN_CLASS(nap::Composition)
	RTTI_PROPERTY("Layers", &nap::Composition::mLayers, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	Composition::~Composition()										{ }


	bool Composition::init(utility::ErrorState& errorState)
	{
		return true;
	}


	nap::BaseTexture2D& Composition::getTexture()
	{
		assert(mLayers.size() > 0);
		return mLayers[0]->getTexture();
	}


	nap::Layer& Composition::getLayer(int index)
	{
		assert(index < getLayerCount());
		int idx = math::clamp<int>(index, 0, getLayerCount() - 1);
		return *(mLayers[idx]);
	}

}