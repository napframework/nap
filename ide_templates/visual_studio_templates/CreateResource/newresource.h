#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * $fileinputname$
	 */
	class NAPAPI $fileinputname$ : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~$fileinputname$();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;
	};
}
