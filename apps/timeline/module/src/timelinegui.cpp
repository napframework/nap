// local includes
#include "timelinegui.h"

// External Includes
#include <entity.h>
#include <imgui/imgui.h>

RTTI_BEGIN_CLASS(nap::TimelineGUI)
RTTI_PROPERTY("Timeline", &nap::TimelineGUI::mTimeline, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void TimelineGUI::construct()
	{
		ImGui::PushID(mID.c_str());

		if (ImGui::Begin(mTimeline->mName.c_str()))
		{
			ImGui::End();
		}

		ImGui::PopID();
	}


	std::string TimelineGUI::getName() const
	{
		return mTimeline->mName;
	}
}
