#pragma once

#include "timelinegui.h"

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <rtti/objectptr.h>


namespace nap
{
	/**
	 * Service associated with timeline module.
	 */
	class NAPAPI TimelineService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		TimelineService(ServiceConfiguration* configuration);

		//
		void construct();
	protected:
		std::vector<std::string> mMenus{ "bla", "bla2" };

		virtual bool init(nap::utility::ErrorState & errorState) override;

		virtual void shutdown() override;

		virtual void resourcesLoaded() override;

	private:
		std::map<std::string, bool>								mTimelineToggledMap;
		std::map<std::string, rtti::ObjectPtr<TimelineGUI>>		mTimelineMap;
	};
}