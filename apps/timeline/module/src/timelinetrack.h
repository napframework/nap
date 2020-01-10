#pragma once

// internal includes
#include "keyframe.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <parameternumeric.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI TimelineTrack : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string									mParameterID;
		std::vector<ResourcePtr<KeyFrame>>			mKeyFrames;
	};
}
