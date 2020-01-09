// local includes
#include "timelinegui.h"
#include "napcolors.h"

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
		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		float stepSize = 100.0f;

		// calc width of content in timeline window
		float timelineWidth = stepSize * mTimeline->mDuration;

		// set content width of next window
		ImGui::SetNextWindowContentWidth(timelineWidth);

		// begin window
		if (ImGui::Begin(
			mTimeline->mName.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{
			for (const auto& track : mTimeline->mTracks)
			{
				//
				const float trackHeight = 100.0f;

				// begin track
				if (ImGui::BeginChild(
					track->mID.c_str(), // id
					ImVec2(timelineWidth, trackHeight), // size
					false)) // no border
				{
					// get window drawlist
					auto drawList = ImGui::GetWindowDrawList();

					// get current imgui cursor position
					auto cursorPos = ImGui::GetCursorPos();

					// get window position
					auto windowTopLeft = ImGui::GetWindowPos();

					// calc beginning of timeline graphic
					auto timelineTopLeft = ImVec2(windowTopLeft.x + cursorPos.x, windowTopLeft.y + cursorPos.y);

					// draw background of timeline
					drawList->AddRectFilled(
						timelineTopLeft, // top left position
						ImVec2(timelineTopLeft.x + timelineWidth, timelineTopLeft.y + trackHeight), // bottom right position
						guicolors::black); // color 

					// draw keyframes
					for (const auto& keyFrame : track->mKeyFrames)
					{
						// position of keyframe
						float x = keyFrame->mTime * stepSize;

						drawList->AddLine(
							ImVec2(timelineTopLeft.x + x, timelineTopLeft.y), // top left
							ImVec2(timelineTopLeft.x + x, timelineTopLeft.y + trackHeight), // bottom right
							guicolors::white, // color
							1.0f); // thickness
					}
				}

				// end track
				ImGui::EndChild();
			}
		}

		// end window
		ImGui::End();

		// pop id
		ImGui::PopID();
	}


	std::string TimelineGUI::getName() const
	{
		return mTimeline->mName;
	}
}
