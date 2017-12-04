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
		if (errorState.check(mLayers.empty(), "No layers found in composition: %s", this->mID.c_str()))
			return false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	CompositionInstance::CompositionInstance(Composition& composition) :
		mComposition(&composition)
	{
		for (auto& layer : composition.mLayers)
			mLayerInstances.emplace_back(layer->createInstance());
	}

	void CompositionInstance::update(double deltaTime)
	{
		for (auto& layer_instance : mLayerInstances)
			layer_instance->update(deltaTime);
	}

	LayerInstance& CompositionInstance::getLayer(int index)
	{
		assert(index < getLayerCount());
		int idx = math::clamp<int>(index, 0, getLayerCount() - 1);
		return *(mLayerInstances[idx]);
	}
}