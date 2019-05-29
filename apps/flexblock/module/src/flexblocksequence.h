#pragma once

// External Includes
#include <nap/resource.h>

#include "flexblocksequenceelement.h"

namespace nap
{
	/**
	 * flexblockstancesequence
	 */
	class NAPAPI FlexBlockSequence : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockSequence();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;
	public:
		// properties
		std::vector<ResourcePtr<FlexBlockSequenceElement>> mElements;
	};
}
