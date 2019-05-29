#pragma once

// External Includes
#include <nap/resource.h>

namespace nap
{
	/**
	 * flexblockstance
	 */
	class NAPAPI FlexBlockStance : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~FlexBlockStance();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<float> mInputs = std::vector<float>{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	};
}
