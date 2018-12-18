#pragma once

// Local Includes
#include "composition.h"

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * Bundles a set of composition
	 */
	class CompositionContainer : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~CompositionContainer();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the number of available compositions in this container
		 */
		int count() const						{ return mCompositions.size(); }

		std::vector<Composition*>				mCompositions;								///< List of all available compositions
	};
}
