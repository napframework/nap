#pragma once

// internal includes
#include "timeline.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	/**
	 */
	class NAPAPI TimelineGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		void construct();

		std::string getName() const;
	public:
		ResourcePtr<Timeline> mTimeline;
	};
}
