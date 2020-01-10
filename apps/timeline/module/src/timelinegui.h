#pragma once

// internal includes
#include "timeline.h"

// external includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rtti/objectptr.h>

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

		void setParameters(const std::vector<rtti::ObjectPtr<ParameterFloat>>& parameters);

		std::string getName() const;

		virtual bool init(utility::ErrorState& errorState) override;
	public:
		ResourcePtr<Timeline>							mTimeline;
		std::vector<rtti::ObjectPtr<ParameterFloat>>	mParameters;
	protected:
		std::vector<const char*>						mParameterNames;
	};
}
