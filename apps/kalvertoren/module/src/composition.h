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

		/**
		 * @return the GPU texture that contains the rendered composition
		 */
		nap::BaseTexture2D& getTexture();

		/**
		 * @return the layer @index
		 * @param index index of the layer to fetch
		 */
		Layer& getLayer(int index);

		/**
		 * @return the total number of layers
		 */
		int getLayerCount() const								{ return mLayers.size(); }

		std::vector<nap::ObjectPtr<Layer>> mLayers;				///< All the layers this composition works with
	};
}
