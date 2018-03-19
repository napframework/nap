#pragma once

// Local includes
#include "layer.h"
#include "imagesequencelayer.h"

// External Includes
#include <nap/resourceptr.h>
#include <nap/resource.h>
#include <vector>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 *	Various modes associated with a composition that determine when it's finished
	 */
	enum class CompositionPlayMode : int
	{
		Length		= 0,					///< Time based composition, finishes after a fixed amount of time
		Sequence	= 1						///< Loop based composition, finishes when the image sequence runs out of iterations
	};


	/**
	 * A composition consists out of a set of layers that are blended on top of each other
	 */
	class NAPAPI Composition : public nap::Resource
	{
		RTTI_ENABLE(rtti::Object)
	public:
		virtual ~Composition();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<ResourcePtr<Layer>>	mLayers;									///< All the layers this composition works with
		CompositionPlayMode					mMode = CompositionPlayMode::Length;		///< Property: controls the composition playback mode
		float								mLength = 1.0f;								///< Length of the sequence
	};


	/**
	 * Instance of a Composition. Creates and owns LayerInstance objects for each layer in the composition.
	 * Updates the instances in update().
	 */
	class NAPAPI CompositionInstance
	{
	public:
		enum class EStatus : int
		{
			WaitingForSequence = 0,
			Active,
			Completed
		};
		
		CompositionInstance(Composition& composition);

		/**
		 * Updates all LayerInstances.
		 */
		void update(double deltaTime);

		/**
		 * @return the layer @index
		 * @param index index of the layer to fetch
		 */
		LayerInstance& getLayer(int index);

		/**
		 * @return the total number of layers
		 */
		int getLayerCount() const { return mLayerInstances.size(); }

		/**
		 * Sets the duration
		 * @param scale the scale applied to the time associated with this composition
		 */
		void setDurationScale(float scale);

		/**
		 *	@return the composition progress in a 0-1 range
		 */
		float getProgress() const;

		/**
		 * @return the name of the composition
		 */
		std::string getName() const;

		/**
		 *	@return current status, active or waiting for sequence to finish
		 */
		EStatus getStatus() const;

		/**
		 *	@return the mode of this composition
		 */
		CompositionPlayMode getMode() const;

		/**
		 *	Signal that is emitted when the composition finished playback
		 */
		nap::Signal<CompositionInstance&> finished;

		CompositionInstance(const CompositionInstance&) = delete;
		CompositionInstance& operator=(const CompositionInstance&) = delete;

	private:
		using LayerInstances = std::vector<std::unique_ptr<LayerInstance>>;
		Composition*				mComposition;							///< Back pointer to Composition resource
		LayerInstances				mLayerInstances;						///< All created LayerInstances, owned by this object
		double						mTime = 0.0;						///< Current playback time
		CompositionPlayMode			mMode = CompositionPlayMode::Length;	///< Composition playback mode
		float						mDurationScale = 1.0f;					///< Influences time and therefore how long this composition lasts
		ImageSequenceLayerInstance* mImageSequence = nullptr;				///< Image sequence we track
		EStatus						mStatus = EStatus::Active;				///< Current sequence status

		/**
		 *	Called internally when the composition finishes
		 */
		void signalFinish();

		/**
		 *	Connects to the completed signal of a sequence and wait till it finishes
		 */
		void connectSequence();

		/**
		 * This function is called when a sequence finishes playback
		 * @param sequence the sequence that finished as single loop
		 */
		void onLayerSequenceFinished(ImageSequenceLayerInstance& sequence);
		NSLOT(mLayerSequenceFinishedSlot, ImageSequenceLayerInstance&, onLayerSequenceFinished)
	};

}
