// local includes
#include "timelinegui.h"
#include "napcolors.h"

// External Includes
#include <entity.h>
#include <imgui/imgui.h>

RTTI_BEGIN_CLASS(nap::TimelineGUI)
RTTI_PROPERTY("Timeline", &nap::TimelineGUI::mTimelineHolder, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void TimelineGUI::draw()
	{
		//
		auto& timeline = mTimelineHolder->getTimelineRef();

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		float stepSize = 100.0f;

		// calc width of content in timeline window
		float timelineWidth = stepSize * timeline.mDuration + 150.0f;

		// set content width of next window
		ImGui::SetNextWindowContentWidth(timelineWidth);

		// begin window
		if (ImGui::Begin(
			timeline.mName.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{
			// check if window has focus
			bool windowHasFocus = ImGui::IsWindowFocused();

			//
			if (ImGui::Button("Save"))
			{
				utility::ErrorState errorState;
				timeline.save("test.json", errorState);
			}

			for (const auto& track : timeline.mTracks)
			{
				// push id
				ImGui::PushID(track->mID.c_str());

				//
				const float trackHeight = 100.0f;
				const float keyframeHandlerHeight = 10.0f;

				// begin track
				if (ImGui::BeginChild(
					track->mID.c_str(), // id
					ImVec2(timelineWidth, trackHeight + keyframeHandlerHeight), // size
					false, // no border
					ImGuiWindowFlags_NoMove )) // window flags
				{
					// get child focus
					bool childHasFocus = windowHasFocus && ImGui::IsWindowFocused();

					// get window drawlist
					auto drawList = ImGui::GetWindowDrawList();

					// get current imgui cursor position
					auto cursorPos = ImGui::GetCursorPos();

					// get window position
					auto windowTopLeft = ImGui::GetWindowPos();

					// calc beginning of timeline graphic
					auto trackTopLeft = ImVec2(windowTopLeft.x + cursorPos.x, windowTopLeft.y + cursorPos.y);

					// draw background of timeline
					drawList->AddRectFilled(
						trackTopLeft, // top left position
						ImVec2(trackTopLeft.x + timelineWidth, trackTopLeft.y + trackHeight), // bottom right position
						guicolors::black); // color 

					//
					float previousKeyFrameX = 0.0f;

					// draw keyframes
					for (const auto& keyFrame : track->mKeyFrames)
					{
						// position of keyframe
						float x = keyFrame->mTime * stepSize;

						// draw keyframe line
						drawList->AddLine(
							ImVec2(trackTopLeft.x + x, trackTopLeft.y), // top left
							ImVec2(trackTopLeft.x + x, trackTopLeft.y + trackHeight), // bottom right
							guicolors::white, // color
							1.0f); // thickness

						// draw keyframe value handler
						drawList->AddCircle(
							ImVec2(trackTopLeft.x + x, trackTopLeft.y + trackHeight * keyFrame->mValue), // position
							5.0f, // radius
							guicolors::red); // color

						// handle mouse actions for keyframe value
						if( mMouseActionData.currentAction == TimelineGUIMouseActions::NONE && 
							ImGui::IsMouseHoveringRect(
								ImVec2(trackTopLeft.x + x - 5, trackTopLeft.y + trackHeight * keyFrame->mValue - 5), // topleft
								ImVec2(trackTopLeft.x + x + 5, trackTopLeft.y + trackHeight * keyFrame->mValue + 5))) // bottomright
						{
							drawList->AddCircleFilled(
								ImVec2(trackTopLeft.x + x, trackTopLeft.y + trackHeight * keyFrame->mValue), // position
								5.0f, // radius
								guicolors::red); // color

							// if we have a click, initiate a mouse action
							if (!mMouseActionData.mouseWasDown && ImGui::IsMouseDown(0))
							{
								mMouseActionData.mouseWasDown = true;
								mMouseActionData.previousMousePos = ImGui::GetMousePos();
								mMouseActionData.currentAction = TimelineGUIMouseActions::DRAGGING_KEYFRAMEVALUE;
								mMouseActionData.currentObject = keyFrame.get();
							}
						}
						
						// process mouse action for keyframe value
						if (mMouseActionData.currentAction == TimelineGUIMouseActions::DRAGGING_KEYFRAMEVALUE &&
							keyFrame.get() == mMouseActionData.currentObject)
						{
							// draw circle filled
							drawList->AddCircleFilled(
								ImVec2(trackTopLeft.x + x, trackTopLeft.y + trackHeight * keyFrame->mValue), // position
								5.0f, // radius
								guicolors::red); // color

							// calc delta
							float deltaY = ImGui::GetMousePos().y - mMouseActionData.previousMousePos.y;

							// translate delta to timeline position
							float translatedValue = keyFrame->mValue + deltaY / trackHeight;

							// clamp value
							translatedValue = math::clamp<float>(translatedValue, 0, 1);

							// set value
							keyFrame->adjustValue(translatedValue);

							//
							mMouseActionData.previousMousePos = ImGui::GetMousePos();

							// handle release of mouse, stop mouse action
							if (mMouseActionData.mouseWasDown && ImGui::IsMouseReleased(0))
							{
								mMouseActionData.mouseWasDown = false;
								mMouseActionData.previousMousePos = ImGui::GetMousePos();
								mMouseActionData.currentAction = TimelineGUIMouseActions::NONE;
								mMouseActionData.currentObject = nullptr;
							}
						}

						// draw curve
						float curveWidth = x - previousKeyFrameX;
						const int resolution = 20;
						std::vector<ImVec2> points;
						points.resize(resolution+1);
						for (int i = 0; i <= resolution; i++)
						{
							float value = keyFrame->mCurve->evaluate((float)i / resolution);

							points[i] = ImVec2(
								trackTopLeft.x + previousKeyFrameX + curveWidth * ((float)i / resolution), 
								trackTopLeft.y + value * trackHeight);
						}

						// draw points of curve
						drawList->AddPolyline(
							&*points.begin(), 
							points.size(),
							guicolors::red,
							false, 
							1.0f, 
							true);

						// keyframe handler box coordinates
						ImVec2 handlerBoxTopLeft = ImVec2(trackTopLeft.x + x, trackTopLeft.y + trackHeight);
						ImVec2 handlerBoxBottomRight = ImVec2(trackTopLeft.x + x + keyframeHandlerHeight, trackTopLeft.y + trackHeight + keyframeHandlerHeight);

						// check if handler is being hovered
						if ( ImGui::IsMouseHoveringRect(handlerBoxTopLeft, handlerBoxBottomRight) && 
							 mMouseActionData.currentAction == TimelineGUIMouseActions::NONE)
						{
							drawList->AddRectFilled(
								handlerBoxTopLeft, // topleft
								handlerBoxBottomRight, // bottomright
								guicolors::white // color
							);

							// if we have a click, initiate a mouse action
							if (!mMouseActionData.mouseWasDown && ImGui::IsMouseDown(0))
							{
								mMouseActionData.mouseWasDown = true;
								mMouseActionData.previousMousePos = ImGui::GetMousePos();
								mMouseActionData.currentAction = TimelineGUIMouseActions::DRAGGING_KEYFRAME;
								mMouseActionData.currentObject = keyFrame.get();
							}
						}

						// handle mouse action
						if (mMouseActionData.currentAction == TimelineGUIMouseActions::DRAGGING_KEYFRAME &&
							mMouseActionData.currentObject == keyFrame.get())
						{
							drawList->AddRectFilled(
								handlerBoxTopLeft, // topleft
								handlerBoxBottomRight, // bottom right
								guicolors::white // color
							);

							// handle mouse drag
							if (mMouseActionData.mouseWasDown && ImGui::IsMouseDragging(0))
							{
								// calc delta
								float deltaX = ImGui::GetMousePos().x - mMouseActionData.previousMousePos.x;

								// translate delta to timeline position
								float timelinepos = keyFrame->mTime + deltaX / stepSize;

								// set keyframe time
								keyFrame->mTime = timelinepos;

								// set previous mouse pos
								mMouseActionData.previousMousePos = ImGui::GetMousePos();
							}

							// handle release of mouse, stop mouse action
							if (mMouseActionData.mouseWasDown && ImGui::IsMouseReleased(0))
							{
								mMouseActionData.mouseWasDown = false;
								mMouseActionData.previousMousePos = ImGui::GetMousePos();
								mMouseActionData.currentAction = TimelineGUIMouseActions::NONE;
								mMouseActionData.currentObject = nullptr;
							}
						}
						else
						{
							drawList->AddRect(
								handlerBoxTopLeft, // topleft
								handlerBoxBottomRight, // bottom right
								guicolors::white // color
							);
						}

						previousKeyFrameX = x;
					}
				}

				// end track
				ImGui::EndChild();

				// pop id
				ImGui::PopID();
			}
		}

		// end window
		ImGui::End();

		// pop id
		ImGui::PopID();
	}


	std::string TimelineGUI::getName() const
	{
		auto& timeline = mTimelineHolder->getTimelineRef();
		return timeline.mName;
	}


	bool nap::TimelineGUI::init(utility::ErrorState & errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		return true;
	}
}
