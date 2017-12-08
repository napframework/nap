#pragma once

// Local includes
#include "layer.h"

// External Includes
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <vector>
#include <nap/signalslot.h>

namespace nap
{
	/**
	 *	Various modes associated with a composition that determine when it's finished
	 */
	enum class CompositionPlayMode : int
	{
		Length		= 0,
		Sequence	= 1
	};


	/**
	 * A composition consists out of a set of layers that are blended on top of each other
	 */
	class NAPAPI Composition : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual ~Composition();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<nap::ObjectPtr<Layer>>	mLayers;									///< All the layers this composition works with
		CompositionPlayMode					mMode = CompositionPlayMode::Length;		///< Property: controls the composition playback mode
		float								mLength = 1.0f;								///< Length of the sequence
		int									mLoopCount = 1;								///< Amount of times the sequences it allowed to loop
	};


	/**
	 * Instance of a Composition. Creates and owns LayerInstance objects for each layer in the composition.
	 * Updates the instances in update().
	 */
	class NAPAPI CompositionInstance
	{
	public:
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
		 *	Signal that is emitted when the composition finished playback
		 */
		nap::Signal<CompositionInstance&> finished;

		CompositionInstance(const CompositionInstance&) = delete;
		CompositionInstance& operator=(const CompositionInstance&) = delete;

	private:
		using LayerInstances = std::vector<std::unique_ptr<LayerInstance>>;
		Composition*		mComposition;							///< Back pointer to Composition resource
		LayerInstances		mLayerInstances;						///< All created LayerInstances, owned by this object
		double				mTime = 0.0;							///< Current playback time
		CompositionPlayMode	mMode = CompositionPlayMode::Length;	///< Composition playback mode
		float				mDurationScale = 1.0f;					///< Influences time and therefore how long this composition lasts
	};

}
