// Local Includes
#include "timelineservice.h"
#include "timelinegui.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>

#include <imgui/imgui.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TimelineService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	TimelineService::TimelineService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ 
	}

	void TimelineService::construct()
	{
		// construct menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Timelines", true))
			{
				for (const auto& pair : mTimelineMap)
				{
					ImGui::PushID(pair.second->mID.c_str());
					ImGui::MenuItem(
						pair.second->getName().c_str(),
						(const char*)0,
						&mTimelineToggledMap[pair.second->mID]);
					ImGui::PopID();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}


		// construct timeline guis
		for (const auto& pair : mTimelineToggledMap)
		{
			if (pair.second)
			{
				mTimelineMap[pair.first]->draw();
			}
		}
	}

	bool TimelineService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void TimelineService::resourcesLoaded()
	{
		// fetch all timeline guis
		auto timelines = getCore().getResourceManager()->getObjects<TimelineGUI>();

		// construct maps
		for (const auto& timelineGUI : timelines)
		{
			// construct maps
			if (mTimelineToggledMap.find(timelineGUI->mID) == mTimelineToggledMap.end())
			{
				mTimelineToggledMap.insert(std::pair<std::string, bool>(timelineGUI->mID, false));
				mTimelineMap.insert(std::pair<std::string, rtti::ObjectPtr<TimelineGUI>>(timelineGUI->mID, timelineGUI));
			}

		}
	}


	void TimelineService::shutdown()
	{
	}
}
