// Local Includes
#include "sequenceeditorservice.h"
#include "sequencegui.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <sceneservice.h>

#include <imgui/imgui.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceEditorService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SequenceEditorService::SequenceEditorService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ 
	}

	void SequenceEditorService::construct()
	{
		// construct menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Sequence Editors", true))
			{
				for (const auto& pair : mTimelineMap)
				{
					ImGui::PushID(pair.second->mID.c_str());
					ImGui::MenuItem(
						pair.second->mID.c_str(),
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
				//mTimelineMap[pair.first]->draw();
			}
		}
	}

	bool SequenceEditorService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void SequenceEditorService::resourcesLoaded()
	{
		// fetch all timeline guis
		auto timelines = getCore().getResourceManager()->getObjects<SequenceGUI>();

		// construct maps
		for (const auto& timelineGUI : timelines)
		{
			// construct maps
			if (mTimelineToggledMap.find(timelineGUI->mID) == mTimelineToggledMap.end())
			{
				mTimelineToggledMap.insert(std::pair<std::string, bool>(timelineGUI->mID, false));
				mTimelineMap.insert(std::pair<std::string, rtti::ObjectPtr<SequenceGUI>>(timelineGUI->mID, timelineGUI));
			}

		}
	}


	void SequenceEditorService::shutdown()
	{
	}
}
