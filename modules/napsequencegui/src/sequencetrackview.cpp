/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "sequencetrackview.h"
#include "sequenceeditorgui.h"
#include "sequenceeditorguiactions.h"

// External Includes
#include <imgui/imgui.h>
#include <iomanip>
#include <imguiutils.h>

using namespace nap::sequenceguiactions;

namespace nap
{
	SequenceTrackView::SequenceTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state) :
		mView(view), mState(state), mService(view.getService())
	{

	}


	static bool vector_getter(void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	}


	bool SequenceTrackView::Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}


	bool SequenceTrackView::ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}


	std::string SequenceTrackView::formatTimeString(double time)
	{
		int hours = (int)(time / 3600.0f);
		int minutes = (int)(time / 60.0f) % 60;
		int seconds = (int)time % 60;
		int milliseconds = (int)(time * 100.0f) % 100;

		return hours == 0 ? utility::stringFormat("%.02d:%.02d:%.02d", minutes, seconds, milliseconds) :
			utility::stringFormat("%.02d:%.02d:%.02d:%.02d", hours, minutes, seconds, milliseconds);
	}


	void SequenceTrackView::showInspector(const SequenceTrack& track)
	{
		bool delete_track = false;
		bool move_track_up = false;
		bool move_track_down = false;
        const float track_height = track.mTrackHeight * mState.mScale;

		// begin inspector
		std::ostringstream inspector_id_stream;
		inspector_id_stream << track.mID << "inspector";
		std::string inspector_id = inspector_id_stream.str();

		// manually set the cursor position before drawing new track window
		ImVec2 cursor_pos = ImGui::GetCursorPos();

		// manually set the cursor position before drawing inspector
		ImVec2 inspector_cursor_pos = cursor_pos;

		// draw inspector window
		float offset = 5.0f * mState.mScale;

		// Push style
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.0f, 0.0f, 0.0f, 0.0f });

		if (ImGui::BeginChild(	inspector_id.c_str(), // id
                                { mState.mInspectorWidth , track_height + offset }, // size
                                false, // no border
                                ImGuiWindowFlags_NoMove)) // window flags
		{
            // push track id
            ImGui::PushID(track.mID.c_str());

			// obtain drawlist
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// store window size and position
			const ImVec2 window_pos = ImGui::GetWindowPos();
			const ImVec2 window_size = ImGui::GetWindowSize();

			// draw background & box
			draw_list->AddRectFilled
			(
				window_pos,
				{window_pos.x + window_size.x - offset, window_pos.y + track_height },
				ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg])
			);

			draw_list->AddRect
			(
				window_pos,
				{ window_pos.x + window_size.x - offset, window_pos.y + track_height },
				mService.getColors().mFro1
			);

            /**
             * Helper function for offsetting inspector
             */
            static std::function<ImVec2(float)> offset_inspector = [](float offset)
            {
                ImVec2 inspector_cursor_pos = ImGui::GetCursorPos();
                inspector_cursor_pos.x += offset;
                inspector_cursor_pos.y += offset;
                ImGui::SetCursorPos(inspector_cursor_pos);

                return inspector_cursor_pos;
            };

			// offset inspector cursor
			inspector_cursor_pos = offset_inspector(offset);

			// scale down everything
			float global_scale = 0.25f;
			ImGui::GetStyle().ScaleAllSizes(global_scale);

			// Remove background
			ImVec4 frame_bg = { 0.0f, 0.0f, 0.0f, 0.0f };
			ImGui::PushStyleColor(ImGuiCol_FrameBg, frame_bg);
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive, frame_bg);
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, frame_bg);

            // show name label
            std::string name_copy = track.mName;
            name_copy.resize(32);
            ImGui::PushID("name_label");
            if(ImGui::InputText("", &name_copy[0], 32))
            {
                mState.mAction = createAction<ChangeTrackName>(track.mID, name_copy);
            }
            ImGui::PopID();

            // offset inspector cursor
            inspector_cursor_pos = offset_inspector(offset);

			// show inspector content
			showInspectorContent(track);

			ImGui::Spacing();

			// delete track button
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
			auto& gui = mService.getGui();
			if (ImGui::ImageButton(gui.getIcon(icon::sequencer::up), "Move track up"))
			{
				move_track_up = true;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton(gui.getIcon(icon::sequencer::down), "Move track down"))
			{
				move_track_down = true;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton(gui.getIcon(icon::del), "Delete track"))
			{
				delete_track = true;
			}

			// Pop gui style elements
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::GetStyle().ScaleAllSizes(1.0f / global_scale);

            // pop id
            ImGui::PopID();
		}
		ImGui::EndChild();

		// Pop background style
		ImGui::PopStyleColor();

		ImGui::SetCursorPos(cursor_pos);

        /**
         * Create necessary action handled by GUI after it's done drawing
         */
		if (delete_track)
        {
            mState.mAction = createAction<DeleteTrack>(track.mID);
        }
		if(move_track_up)
		{
            mState.mAction = createAction<MoveTrackUp>(track.mID);
		}
		if(move_track_down)
		{
            mState.mAction = createAction<MoveTrackDown>(track.mID);
		}
	}


	void SequenceTrackView::showTrack(const SequenceTrack& track)
	{
		ImVec2 cursor_pos = ImGui::GetCursorPos();
		float offset = 5.0f * mState.mScale;
		const ImVec2 window_cursor_pos = { cursor_pos.x + offset, cursor_pos.y };

		ImGui::SetCursorPos(window_cursor_pos);

		// begin track
		ImGui::PushStyleColor(ImGuiCol_ChildBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mState.mTimelineWidth + offset, (track.mTrackHeight * mState.mScale) + (10.0f * mState.mScale) }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			// push id
			ImGui::PushID(track.mID.c_str());

			// get window drawlist
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// get current imgui cursor position
			cursor_pos = ImGui::GetCursorPos();

			// get window position
			ImVec2 window_top_left = ImGui::GetWindowPos();

			// calc beginning of timeline graphic
			ImVec2 track_top_left = {window_top_left.x + cursor_pos.x, window_top_left.y + cursor_pos.y };

            ImVec2 track_bottom_right = {track_top_left.x + mState.mTimelineWidth, track_top_left.y + (track.mTrackHeight * mState.mScale)};

			// draw background of track
			draw_list->AddRectFilled(track_top_left, track_bottom_right,
				ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_FrameBg])); // color

			// draw border of track
			draw_list->AddRect(
                    track_top_left, // top left position
                    track_bottom_right, // bottom right position
				mService.getColors().mFro1); // color

			// draw timestamp every 100 pixels
			const float timestamp_interval = 100.0f;
			int steps = mState.mTimelineWidth / timestamp_interval;

			int i = ( math::max<int>(mState.mScroll.x - mState.mInspectorWidth + (100.0f * mState.mScale), 0) / timestamp_interval);
			for (;i < steps; i++)
			{
				if(i==0) // ignore first timestamp since it will hide window left border
					continue;

				ImVec2 pos = {track_top_left.x + i * timestamp_interval, track_top_left.y };

				if (pos.x > 0.0f )
				{
					draw_list->AddLine(pos, { pos.x, pos.y + (track.mTrackHeight * mState.mScale) }, mService.getColors().mBack);
					if(pos.x > mState.mWindowPos.x + mState.mScroll.x + mState.mWindowSize.x)
						break;
				}
			}

			mState.mMouseCursorTime = (mState.mMousePos.x - track_top_left.x) / mState.mStepSize;
			showTrackContent(track, track_top_left);

            // extension handler of track height
            if(mState.mAction->isAction<None>())
            {
                // are we hovering bottom of the track ?
                if(ImGui::IsMouseHoveringRect({ track_top_left.x, track_bottom_right.y - 5.0f * mState.mScale },
                                              { track_bottom_right.x, track_bottom_right.y + 5.0f * mState.mScale }))
                {
                    // draw line to indicate we are hovering
                    draw_list->AddLine({ track_top_left.x, track_bottom_right.y }, { track_bottom_right.x, track_bottom_right.y },  mService.getColors().mFro1, 4.0f * mState.mScale);

                    if(ImGui::IsMouseDown(0))
                    {
                        mState.mAction = createAction<DraggingTrackExtensionHandler>(track.mID);
                    }
                }
            }else if(mState.mAction->isAction<DraggingTrackExtensionHandler>())
            {
                auto* action = mState.mAction->getDerived<DraggingTrackExtensionHandler>();
                if(action->mTrackID==track.mID)
                {
                    // draw line to indicate we are dragging
                    draw_list->AddLine({ track_top_left.x, track_bottom_right.y }, { track_bottom_right.x, track_bottom_right.y },  mService.getColors().mFro1, 4.0f * mState.mScale);
                }
            }
			// pop id
			ImGui::PopID();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
		ImGui::SetCursorPos({cursor_pos.x, cursor_pos.y } );
	}


	void SequenceTrackView::handleActions()
	{
		// check if there is a track action
		if(mState.mAction.get()->get_type().is_derived_from<TrackAction>())
		{
			// get derived track action
			assert(mState.mAction.get()->get_type().is_derived_from<TrackAction>());
			const auto* track_action = static_cast<const TrackAction*>(mState.mAction.get());

			// find the track this track action belongs to
			const auto& sequence = getEditor().mSequencePlayer->getSequenceConst();
			for(const auto& track : sequence.mTracks)
			{
				const auto* track_ptr = track.get();
				if(track_ptr->mID == track_action->mTrackID)
				{
					// does the track type match for this view?
					rtti::TypeInfo view_type_for_track = mService.getViewTypeForTrackType(track_ptr->get_type());
					rtti::TypeInfo current_view_type = get_type();

					if(view_type_for_track == current_view_type)
					{
						// handle any action if necessary
						auto it = mActionHandlers.find(mState.mAction.get()->get_type());
						if(it != mActionHandlers.end())
						{
							it->second();
						}
					}

					break;
				}
			}
		}
	}


	const SequencePlayer& SequenceTrackView::getPlayer() { return *mView.mEditor.mSequencePlayer.get(); }

	
	SequenceEditor& SequenceTrackView::getEditor() { return mView.mEditor; }


	void SequenceTrackView::registerActionHandler(const rttr::type& type, const std::function<void()>& handler)
	{
		// Assert is triggered when element with same key already exists
		auto it = mActionHandlers.emplace(std::make_pair(type, handler));
		assert(it.second);
	}
}
