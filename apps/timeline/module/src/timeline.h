#pragma once

// internal includes
#include "keyframe.h"
#include "timelinetrack.h"

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
		std::vector<ResourcePtr<TimelineTrack>> mTracks;
		
		bool init(utility::ErrorState& errorState) override;

		double mDuration = 1.0;
	protected:
	};
}
