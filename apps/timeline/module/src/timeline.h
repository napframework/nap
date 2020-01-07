#pragma once

// internal includes
#include "keyframe.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI Timeline : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string								mName = "Timeline";
		std::vector<ResourcePtr<Parameter>>		mParameters;
		std::vector<ResourcePtr<KeyFrame>>		mKeyFrames;

		bool init(utility::ErrorState& errorState) override;
	protected:
	};
}
