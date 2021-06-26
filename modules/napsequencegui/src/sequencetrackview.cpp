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

using namespace nap::SequenceGUIActions;

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

		// begin inspector
		std::ostringstream inspector_id_stream;
		inspector_id_stream << track.mID << "inspector";
		std::string inspector_id = inspector_id_stream.str();

		// manually set the cursor position before drawing new track window
		ImVec2 cursor_pos =
			{
				ImGui::GetCursorPosX() ,
				ImGui::GetCursorPosY()
			};

		// manually set the cursor position before drawing inspector
		ImVec2 inspector_cursor_pos = {cursor_pos.x , cursor_pos.y };
		ImGui::SetCursorPos(inspector_cursor_pos);

		// draw inspector window
		if (ImGui::BeginChild(	inspector_id.c_str(), // id
								{ mState.mInspectorWidth , mState.mTrackHeight + 5 }, // size
	  							false, // no border
	   							ImGuiWindowFlags_NoMove)) // window flags
		{
			// obtain drawlist
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// store window size and position
			const ImVec2 window_pos = ImGui::GetWindowPos();
			const ImVec2 window_size = ImGui::GetWindowSize();

			// draw background & box
			draw_list->AddRectFilled(	window_pos,
							  			{window_pos.x + window_size.x - 5, window_pos.y + mState.mTrackHeight },
									 	sequencer::colors::black);

			draw_list->AddRect(	window_pos,
					    		{window_pos.x + window_size.x - 5, window_pos.y + mState.mTrackHeight },
							   	sequencer::colors::white);

			//
			inspector_cursor_pos = ImGui::GetCursorPos();
			inspector_cursor_pos.x += 5;
			inspector_cursor_pos.y += 5;
			ImGui::SetCursorPos(inspector_cursor_pos);

			// scale down everything
			float scale = 0.25f;
			ImGui::GetStyle().ScaleAllSizes(scale);

			//
			showInspectorContent(track);

			// delete track button
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

			// when we delete a track, we don't immediately call the controller because we are iterating track atm
			if (ImGui::SmallButton("Delete"))
			{
				delete_track = true;
			}

			// show up & down buttons
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

			if(ImGui::SmallButton("Up"))
			{
				move_track_up = true;
			}
			ImGui::SameLine();
			if(ImGui::SmallButton("Down"))
			{
				move_track_down = true;
			}

			// pop scale
			ImGui::GetStyle().ScaleAllSizes(1.0f / scale);
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(cursor_pos);

		if (delete_track)
		{
			auto* controller = getEditor().getControllerWithTrackType(track.get_type());
			assert(controller!= nullptr); // controller not found
			if(controller!= nullptr)
			{
				controller->deleteTrack(track.mID);
				mState.mDirty = true;
			}
		}

		if(move_track_up)
		{
			auto* controller = getEditor().getControllerWithTrackType(track.get_type());
			assert(controller!= nullptr); // controller not found
			if(controller!= nullptr)
			{
				controller->moveTrackUp(track.mID);
				mState.mDirty = true;
			}
		}

		if(move_track_down)
		{
			auto* controller = getEditor().getControllerWithTrackType(track.get_type());
			assert(controller!= nullptr); // controller not found
			if(controller!= nullptr)
			{
				controller->moveTrackDown(track.mID);
				mState.mDirty = true;
			}
		}
	}


	void SequenceTrackView::showTrack(const SequenceTrack& track)
	{
		ImVec2 cursor_pos = ImGui::GetCursorPos();

		const ImVec2 window_cursor_pos = {cursor_pos.x + 5, cursor_pos.y };
		ImGui::SetCursorPos(window_cursor_pos);

		// begin track
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mState.mTimelineWidth + 5 , mState.mTrackHeight + 10 }, // size
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
			ImVec2 trackTopLeft = {window_top_left.x + cursor_pos.x, window_top_left.y + cursor_pos.y };

			// draw background of track
			draw_list->AddRectFilled(
				trackTopLeft, // top left position
				{ trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + mState.mTrackHeight }, // bottom right position
				sequencer::colors::black); // color

			// draw border of track
			draw_list->AddRect(
				trackTopLeft, // top left position
				{ trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + mState.mTrackHeight }, // bottom right position
				sequencer::colors::white); // color

			// draw timestamp every 100 pixels
			const float timestamp_interval = 100.0f;
			int steps = mState.mTimelineWidth / timestamp_interval;

			int i = ( math::max<int>(mState.mScroll.x - mState.mInspectorWidth + 100, 0) / timestamp_interval);
			for (;i < steps; i++)
			{
				if(i==0) // ignore first timestamp since it will hide window left border
					continue;

				ImVec2 pos = { trackTopLeft.x + i * timestamp_interval, trackTopLeft.y };

				if (pos.x > 0.0f )
				{
					draw_list->AddLine(pos, { pos.x, pos.y + mState.mTrackHeight }, sequencer::colors::darkerGrey);

					if(pos.x > mState.mWindowPos.x + mState.mScroll.x + mState.mWindowSize.x)
					{
						break;
					}
				}
			}

			mState.mMouseCursorTime = (mState.mMousePos.x - trackTopLeft.x) / mState.mStepSize;

			showTrackContent(track, trackTopLeft);

			// pop id
			ImGui::PopID();
		}

		ImGui::EndChild();
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
