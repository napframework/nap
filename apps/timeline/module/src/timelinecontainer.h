#pragma once

// internal includes
#include "timeline.h"

// external includes
#include <nap/resource.h>
#include <parameter.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class TimelineGUI;

	/**
	 * TimelineHolder
	 * TimelineHolder is responsible for instantiating a timeline, and the let it load and or save a show from/to disk
	 */
	class NAPAPI TimelineContainer : public Resource
	{
		friend class TimelineGUI;

		RTTI_ENABLE(Resource)
	public:
		// Properties
		std::string	mTimelineFilePath; ///< The default filepath that the timeline should load on startup
	public:
		// Functions

		/**
		 * 
		 */
		bool init(utility::ErrorState& errorState) override;

		bool save(const std::string& name, utility::ErrorState& errorState);

		bool load(const std::string& name, utility::ErrorState& errorState);
	protected:
		std::unique_ptr<Timeline>	mTimeline = nullptr;
	};
}
