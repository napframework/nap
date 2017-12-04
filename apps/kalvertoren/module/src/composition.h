#pragma once

// Local includes
#include "layer.h"

// External Includes
#include <rtti/rttiobject.h>
#include <nap/objectptr.h>
#include <vector>

namespace nap
{
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

		std::vector<nap::ObjectPtr<Layer>> mLayers;				///< All the layers this composition works with
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

		CompositionInstance(const CompositionInstance&) = delete;
		CompositionInstance& operator=(const CompositionInstance&) = delete;


	private:
		using LayerInstances = std::vector<std::unique_ptr<LayerInstance>>;
		Composition*		mComposition;			///< Back pointer to Composition resource
		LayerInstances		mLayerInstances;		///< All created LayerInstances, owned by this object
	};

}
