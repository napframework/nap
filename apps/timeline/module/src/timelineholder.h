#pragma once

// internal includes
#include "timeline.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	*/
	class NAPAPI TimelineHolder : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		std::string								mTimelineFilePath;

		bool init(utility::ErrorState& errorState) override;

		bool save(const std::string& name, utility::ErrorState& errorState);

		bool load(const std::string& name, utility::ErrorState& errorState);

		Timeline& getTimelineRef() { return *mTimeline; }
	protected:
		std::unique_ptr<Timeline>	mTimeline = nullptr;
	};
}
