#pragma once

// External Includes
#include <nap/resource.h>
#include <parameter.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * A group of Parameters that are automatically blended over time.
	 */
	class NAPAPI ParameterBlendGroup : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~ParameterBlendGroup();

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<ResourcePtr<Parameter>> mParameters;						///< Property: 'Parameters' list of all parameters considered to blend
		nap::ResourcePtr<ParameterGroup> mParameterGroup = nullptr;				///< Property: 'ParameterGroup' group all the blend parameters belong to
	};
}
