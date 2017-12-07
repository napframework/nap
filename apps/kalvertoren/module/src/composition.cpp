#include "composition.h"

#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::CompositionPlayMode)
	RTTI_ENUM_VALUE(nap::CompositionPlayMode::Length,	"Length"),
	RTTI_ENUM_VALUE(nap::CompositionPlayMode::Sequence, "Sequence")
RTTI_END_ENUM

// nap::composition run time class definition 
RTTI_BEGIN_CLASS(nap::Composition)
	RTTI_PROPERTY("Layers", &nap::Composition::mLayers,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length", &nap::Composition::mLength,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Loops",	&nap::Composition::mLoopCount,	nap::rtti::EPropertyMetaData::Default)
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
		{
			mLayerInstances.emplace_back(layer->createInstance());
		}

		// TODO: Allow looping when one of the layers is a sequence
		// Revert back to time based playback when none of the layers is an actual sequence
		mMode = CompositionPlayMode::Length;
	}

	void CompositionInstance::update(double deltaTime)
	{
		// Update all associated layers 
		for (auto& layer_instance : mLayerInstances)
			layer_instance->update(deltaTime);

		// Signal finished when time exceeds current playback time
		if (mTime > mComposition->mLength)
			finished(*this);

		// Increment time
		mTime += deltaTime;
	}

	LayerInstance& CompositionInstance::getLayer(int index)
	{
		assert(index < getLayerCount());
		int idx = math::clamp<int>(index, 0, getLayerCount() - 1);
		return *(mLayerInstances[idx]);
	}
}