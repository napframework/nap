#include "composition.h"
#include "imagesequencelayer.h"

#include <mathutils.h>

RTTI_BEGIN_ENUM(nap::CompositionPlayMode)
	RTTI_ENUM_VALUE(nap::CompositionPlayMode::Length,	"Length"),
	RTTI_ENUM_VALUE(nap::CompositionPlayMode::Sequence, "Sequence")
RTTI_END_ENUM

// nap::composition run time class definition 
RTTI_BEGIN_CLASS(nap::Composition)
	RTTI_PROPERTY("Layers", &nap::Composition::mLayers,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Mode",	&nap::Composition::mMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Length", &nap::Composition::mLength,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	Composition::~Composition()										{ }


	bool Composition::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(!mLayers.empty(), "No layers found in composition: %s", this->mID.c_str()))
			return false;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	CompositionInstance::CompositionInstance(Composition& composition) :
		mComposition(&composition)
	{
		mImageSequence = nullptr;
		for (auto& layer : composition.mLayers)
		{
			// Create instance out of every available layer type
			std::unique_ptr<LayerInstance> layer_instance = layer->createInstance();
			
			// If it's an image sequence, we want to store it so we can work with it later
			// The longest sequences take presendence over shorter ones
			if (layer_instance->get_type().is_derived_from(RTTI_OF(ImageSequenceLayerInstance)))
			{
				nap::ImageSequenceLayerInstance* current = rtti_cast<ImageSequenceLayerInstance>(layer_instance.get());
				if (mImageSequence == nullptr || mImageSequence->getLength() < current->getLength())
				{
					mImageSequence = current;
				}
			}
			mLayerInstances.emplace_back(std::move(layer_instance));
		}
		
		// If the user selected sequence mode and there is a sequence available, make that the default
		mMode = composition.mMode == CompositionPlayMode::Sequence && mImageSequence != nullptr ? CompositionPlayMode::Sequence : CompositionPlayMode::Length;
	}


	void CompositionInstance::update(double deltaTime)
	{
		if (mTime > mComposition->mLength)
		{
			switch (mMode)
			{
			case CompositionPlayMode::Length:
				finished(*this);
				break;
			case CompositionPlayMode::Sequence:
				assert(mImageSequence != nullptr);
				mImageSequence->completed.connect(mLayerSequenceFinishedSlot);
				break;
			}
			mTime = 0.0f;
		}


		// Update all associated layers 
		for (auto& layer_instance : mLayerInstances)
			layer_instance->update(deltaTime);

		// Increment time
		mTime += (deltaTime * mDurationScale);
	}


	void CompositionInstance::onLayerSequenceFinished(ImageSequenceLayerInstance& sequence)
	{
		finished(*this);
	}


	LayerInstance& CompositionInstance::getLayer(int index)
	{
		assert(index < getLayerCount());
		int idx = math::clamp<int>(index, 0, getLayerCount() - 1);
		return *(mLayerInstances[idx]);
	}


	void CompositionInstance::setDurationScale(float scale)
	{
		mDurationScale = scale;
	}
}