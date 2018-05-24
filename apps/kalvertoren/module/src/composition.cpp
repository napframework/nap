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

		// Set status
		mStatus = EStatus::Active;
	}


	CompositionInstance::~CompositionInstance()
	{
		mLayerInstances.clear();
	}


	void CompositionInstance::update(double deltaTime)
	{
		// Increment time if this component is active
		if (mStatus == EStatus::Active)
		{
			mTime += (deltaTime * mDurationScale);
		}

		// When time runs out signal completion or, when we're dealing with a sequence:
		// Wait for it to finish and signal completion
		if (mTime > mComposition->mLength)
		{
			switch (mMode)
			{
			case CompositionPlayMode::Length:
				signalFinish();
				break;
			case CompositionPlayMode::Sequence:
				connectSequence();
				break;
			}

			// Reset time
			mTime = 0.0;
		}

		// Update all associated layers 
		for (auto& layer_instance : mLayerInstances)
			layer_instance->update(deltaTime);
	}


	void CompositionInstance::onLayerSequenceFinished(ImageSequenceLayerInstance& sequence)
	{
		signalFinish();
	}

	void CompositionInstance::signalFinish()
	{
		mStatus = EStatus::Completed;
		finished(*this);
	}


	void CompositionInstance::connectSequence()
	{
		mStatus = EStatus::WaitingForSequence;
		mImageSequence->completed.connect(mLayerSequenceFinishedSlot);
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


	float CompositionInstance::getProgress() const
	{
		switch (mStatus)
		{
		case CompositionInstance::EStatus::Active:
			return math::fit<float>(mTime, 0.0f, mComposition->mLength, 0.0f, 1.0f);
		case CompositionInstance::EStatus::WaitingForSequence:
			return mImageSequence->getProgress();
		case CompositionInstance::EStatus::Completed:
			return 1.0f;
		default:
			assert(false);
		}
		return 0.0f;
	}


	std::string CompositionInstance::getName() const
	{
		return mComposition->mID;
	}


	CompositionInstance::EStatus CompositionInstance::getStatus() const
	{
		return mStatus;
	}


	nap::CompositionPlayMode CompositionInstance::getMode() const
	{
		return mMode;
	}

}