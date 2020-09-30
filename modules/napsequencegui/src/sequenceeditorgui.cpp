/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// local includes
#include "sequenceeditorgui.h"
#include "napcolors.h"


// External Includes
#include <entity.h>
#include <imgui/imgui.h>
#include <nap/logger.h>
#include <utility/fileutils.h>
#include <iomanip>

RTTI_BEGIN_CLASS(nap::SequenceEditorGUI)
RTTI_PROPERTY("Sequence Editor", &nap::SequenceEditorGUI::mSequenceEditor, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Render Window", &nap::SequenceEditorGUI::mRenderWindow, nap::rtti::EPropertyMetaData::Required)
RTTI_PROPERTY("Draw Full Window", &nap::SequenceEditorGUI::mDrawFullWindow, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::SequenceGUIActions;
using namespace nap::SequenceCurveEnums;

namespace nap
{
	static std::unordered_map<rttr::type, rttr::type>& getTrackViewTypeViewMap()
	{
		static std::unordered_map<rttr::type, rttr::type> map;
		return map;
	};


	bool SequenceEditorGUIView::registerTrackViewType(rttr::type trackType, rttr::type viewType)
	{
		auto& map = getTrackViewTypeViewMap();
		auto it = map.find(trackType);
		assert(it == map.end()); // duplicate entry
		if (it == map.end())
		{
			map.emplace(trackType, viewType);
			return true;
		}

		return false;
	}


	bool SequenceEditorGUI::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		mView = std::make_unique<SequenceEditorGUIView>(*mSequenceEditor.get(), mID, mRenderWindow.get(), mDrawFullWindow);

		return true;
	}


	void SequenceEditorGUI::onDestroy()
	{
	}


	void SequenceEditorGUI::show()
	{
		mView->show();
	}


	SequenceEditorGUIView::SequenceEditorGUIView(SequenceEditor& editor, std::string id, RenderWindow* renderWindow, bool drawFullWindow)
		: mEditor(editor), mID(id), mRenderWindow(renderWindow), mDrawFullWindow(drawFullWindow)
	{
		mState.mAction = createAction<None>();

		for (auto& factory : SequenceTrackView::getFactoryMap())
		{
			mViews.emplace(factory.first, factory.second(*this, mState));
		}
	}


	void SequenceEditorGUIView::show()
	{
		bool reset_dirty_flag = mState.mDirty;
		mState.mInspectorWidth = 300.0f;		
		mState.mMousePos = ImGui::GetMousePos();

		mState.mMouseDelta = ImGui::GetIO().MouseDelta;

		//
		const Sequence& sequence = mEditor.mSequencePlayer->getSequenceConst();
		SequencePlayer& sequence_player = *mEditor.mSequencePlayer.get();

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		mState.mStepSize = mState.mHorizontalResolution;

		// calc width of content in timeline window
		mState.mTimelineWidth = mState.mStepSize * sequence.mDuration;

		// set content width of next window
		ImGui::SetNextWindowContentSize(ImVec2(mState.mTimelineWidth + mState.mInspectorWidth + mState.mVerticalResolution, 0.0f));

		// set window flags
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		if( mDrawFullWindow )
			window_flags = window_flags | ImGuiWindowFlags_NoResize;

		if( mDrawFullWindow )
		{
			ImGui::SetNextWindowPos({0,0});
			ImGui::SetNextWindowSize({ static_cast<float>(mRenderWindow->getSize().x), static_cast<float>(mRenderWindow->getSize().y) });
		}
		// begin window
		if (ImGui::Begin(
			mID.c_str(), // id
			(bool*)0, // open
			window_flags)) // window flags
		{
			//
			ImVec2 windowSize = ImGui::GetWindowSize();
			if (windowSize.x != mState.mWindowSize.x || windowSize.y != mState.mWindowSize.y)
			{
				mState.mDirty = true;
				mState.mWindowSize = windowSize;
			}

			// we want to know if this window is focused in order to handle mouseinput
			// in child windows or not
			mState.mIsWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

			// clear curve cache if we move the window
			mState.mWindowPos = ImGui::GetWindowPos();
			if (mState.mWindowPos.x != mState.mPrevWindowPos.x ||
				mState.mWindowPos.y != mState.mPrevWindowPos.y)
			{
				mState.mDirty = true;
			}
			mState.mPrevWindowPos = mState.mWindowPos;

			// clear curve cache if we scroll inside the window
			ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
			if (scroll.x != mState.mScroll.x || scroll.y != mState.mScroll.y)
			{
				mState.mDirty = true;
			}
			mState.mScroll = scroll;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetScrollX());

			//
			if (ImGui::Button("Save"))
			{
				mEditor.save(mEditor.mSequencePlayer->mSequenceFileName);
			}

			ImGui::SameLine();

			if (ImGui::Button("Save As"))
			{
				ImGui::OpenPopup("Save As");
				mState.mAction = createAction<SaveAsPopup>();
			}

			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				ImGui::OpenPopup("Load");
				mState.mAction = createAction<LoadPopup>();
			}

			ImGui::SameLine();

			if (sequence_player.getIsPlaying())
			{
				if (ImGui::Button("Stop"))
				{
					sequence_player.setIsPlaying(false);
				}
			}
			else
			{
				if (ImGui::Button("Play"))
				{
					sequence_player.setIsPlaying(true);
				}
			}

			ImGui::SameLine();
			if (sequence_player.getIsPaused())
			{
				if (ImGui::Button("Unpause"))
				{
					sequence_player.setIsPaused(false);
				}
			}
			else
			{
				if (ImGui::Button("Pause"))
				{
					sequence_player.setIsPaused(true);
				}
			}
			

			ImGui::SameLine();
			if (ImGui::Button("Rewind"))
			{
				sequence_player.setPlayerTime(0.0);
			}

			ImGui::SameLine();
			bool isLooping = sequence_player.getIsLooping();
			if (ImGui::Checkbox("Loop", &isLooping))
			{
				sequence_player.setIsLooping(isLooping);
			}

			ImGui::SameLine();
			float playback_speed = sequence_player.getPlaybackSpeed();
			ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("speed", &playback_speed, 0.01f, -10.0f, 10.0f, "%.1f"))
			{
				playback_speed = math::clamp(playback_speed, -10.0f, 10.0f);
				sequence_player.setPlaybackSpeed(playback_speed);
			}
			ImGui::PopItemWidth();

			ImGui::SameLine();

			if (ImGui::Checkbox("Follow", &mState.mFollow))
			{
			}

			if (mState.mFollow)
			{
				float scroll_x = (sequence_player.getPlayerTime() / sequence_player.getDuration()) * mState.mTimelineWidth;
				ImGui::SetScrollX(scroll_x);
			}

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetScrollX());

			ImGui::PushItemWidth(200.0f);
			if (ImGui::DragFloat("H-Zoom", &mState.mHorizontalResolution, 0.5f, 2.5, 1000, "%0.1f"))
				mState.mDirty = true;
			ImGui::SameLine();
			if (ImGui::DragFloat("V-Zoom", &mState.mVerticalResolution, 0.5f, 150, 1000, "%0.1f"))
				mState.mDirty = true;
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// store position of next window ( player controller ), we need it later to draw the timelineplayer position
			mState.mTimelineControllerPos = ImGui::GetCursorPos();

			drawPlayerController(sequence_player);

			// move a little bit more up to align tracks nicely with timelinecontroller
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mState.mVerticalResolution - 10);

			// draw tracks
			drawTracks(sequence_player, sequence);

			// draw end of sequence line
			drawEndOfSequence(sequence, sequence_player);

			//
			drawTimelinePlayerPosition(sequence, sequence_player);

			// move the cursor below the tracks
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mState.mVerticalResolution + 10.0f);
			if (ImGui::Button("Insert New Track"))
			{
				mState.mAction = createAction<OpenInsertTrackPopup>();
			}

			// handle popups & actions
			for (auto& it : mViews)
			{
				it.second->handlePopups();
				it.second->handleActions();
			}

			//
			handleInsertTrackPopup();

			//
			handleLoadPopup();

			//
			handleSaveAsPopup();

			//
			handleSequenceDurationPopup();
		}

		ImGui::End();

		// pop id
		ImGui::PopID();

		if(reset_dirty_flag)
		{
			mState.mDirty = false;
		}
	}


	void SequenceEditorGUIView::drawTracks(const SequencePlayer& sequencePlayer, const Sequence &sequence)
	{
		// get current cursor pos, we will use this to position the track windows
		mState.mCursorPos = ImGui::GetCursorPos();

		// define consts
		mState.mTrackHeight = mState.mVerticalResolution;

		// draw tracks
		for(int i = 0; i < sequence.mTracks.size(); i++)
		{
			auto track_type = sequence.mTracks[i].get()->get_type();
			auto view_map = getTrackViewTypeViewMap();
			auto it = view_map.find(track_type);
			assert(it != view_map.end()); // no view type for track
			if (it != view_map.end())
			{
				auto it2 = mViews.find(it->second);
				assert(it2 != mViews.end()); // no view class created for this view type
				if (it2 != mViews.end())
				{
					it2->second->show(*sequence.mTracks[i].get());
				}
			}
		}
	}


	void SequenceEditorGUIView::drawPlayerController(SequencePlayer& player)
	{
		const float sequence_controller_height = 30.0f;

		std::ostringstream string_stream;
		string_stream << mID << "sequencecontroller";
		std::string id_string = string_stream.str();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + mState.mInspectorWidth + 5.0f);
		ImGui::PushID(id_string.c_str());

		// used for culling ( is stuff inside the parent window ??? )
		ImVec2 parent_window_pos = ImGui::GetWindowPos();
		ImVec2 parent_window_size = ImGui::GetWindowSize();

		// draw timeline controller
		if (ImGui::BeginChild(id_string.c_str(), // id
			{ mState.mTimelineWidth + 5 , sequence_controller_height}, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			ImVec2 cursor_pos	 = ImGui::GetCursorPos();
			ImVec2 window_top_left = ImGui::GetWindowPos();
			ImVec2 start_pos	   =
			{
				   window_top_left.x + cursor_pos.x,
				   window_top_left.y + cursor_pos.y + 15,
			};

			cursor_pos.y += 5;

			// get window drawlist
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			// draw backgroundbox of controller
			draw_list->AddRectFilled(
				start_pos,
				{start_pos.x + mState.mTimelineWidth, start_pos.y + sequence_controller_height - 15 }, guicolors::black);

			// draw box of controller
			draw_list->AddRect(start_pos,
				{start_pos.x + mState.mTimelineWidth, start_pos.y + sequence_controller_height - 15}, guicolors::white);

			// draw handler of player position
			const double player_time = player.getPlayerTime();
			const ImVec2 player_time_top_rect_left =
			{
				start_pos.x + (float)(player_time / player.getDuration()) * mState.mTimelineWidth - 5, start_pos.y
			};
			const ImVec2 player_time_rect_bottom_right =
			{
				start_pos.x + (float)(player_time / player.getDuration()) * mState.mTimelineWidth + 5,
				start_pos.y + sequence_controller_height,
			};

			draw_list->AddRectFilled(player_time_top_rect_left, player_time_rect_bottom_right,
				guicolors::red);

			// draw timestamp text every 100 pixels
			const float timestamp_interval = 100.0f;
			int steps = mState.mTimelineWidth / timestamp_interval;
			int step_start = math::max<float>(mState.mScroll.x - start_pos.x, start_pos.x) / mState.mTimelineWidth;
			double step_size = player.getDuration() / mState.mTimelineWidth;
			for (int i = step_start; i < steps; i++)
			{
				ImVec2 timestamp_pos;
				timestamp_pos.x = i * timestamp_interval + start_pos.x;
				timestamp_pos.y = start_pos.y - 18;

				if (timestamp_pos.x < parent_window_size.x + parent_window_pos.x &&
					timestamp_pos.x >= parent_window_pos.x)
				{
					if (timestamp_pos.y >= parent_window_pos.y &&
						timestamp_pos.y < parent_window_size.y + parent_window_pos.y)
					{
						double time_in_player = step_size * i;
						std::string formatted_time_string = SequenceTrackView::formatTimeString(time_in_player);
						draw_list->AddText(timestamp_pos, guicolors::white, formatted_time_string.c_str());

						if (i != 0)
						{
							draw_list->AddLine({timestamp_pos.x, timestamp_pos.y + 18 }, {timestamp_pos.x, timestamp_pos.y + sequence_controller_height + 2 }, guicolors::darkGrey);
						}
					}
				}
				else
				{
					if (timestamp_pos.x > parent_window_size.x + parent_window_pos.x)
						break; // right side of window, break out of loop
				}
			}

			// handle hovering and dragging of player position
			if (mState.mIsWindowFocused)
			{
				if (mState.mAction->isAction<None>()|| mState.mAction->isAction<HoveringPlayerTime>())
				{
					if (ImGui::IsMouseHoveringRect(start_pos, {start_pos.x + mState.mTimelineWidth, start_pos.y + sequence_controller_height}))
					{
						mState.mAction = createAction<HoveringPlayerTime>();

						if (ImGui::IsMouseDown(0))
						{
							//
							bool player_was_playing = player.getIsPlaying();
							bool player_was_paused	= player.getIsPaused();

							mState.mAction = createAction<DraggingPlayerTime>(player_was_playing, player_was_paused);
							if (player_was_playing)
							{
								player.setIsPaused(true);
							}
							
							// snap to mouse position
							double time = ((ImGui::GetMousePos().x - start_pos.x) / mState.mTimelineWidth) * player.getDuration();
							player.setPlayerTime(time);
						}
					}
					else
					{
						mState.mAction = createAction<None>();
					}
				}else if (mState.mAction->isAction<DraggingPlayerTime>())
				{
					if (ImGui::IsMouseDown(0))
					{
						double delta = (mState.mMouseDelta.x / mState.mTimelineWidth) * player.getDuration();
						player.setPlayerTime(player_time + delta);
					}
					else
					{
						if (ImGui::IsMouseReleased(0))
						{
							const auto* drag_action = mState.mAction->getDerived<DraggingPlayerTime>();
							assert(drag_action != nullptr); 
							if (drag_action->mWasPlaying && !drag_action->mWasPaused)
							{
								player.setIsPlaying(true);
							}

							mState.mAction = createAction<None>();
						}
					}
				}
			}

			// handle dragging of timeline duration
			const double player_duration = player.getDuration();
			const ImVec2 player_duration_top_rect_left =
			{
				start_pos.x +  mState.mTimelineWidth - 5, start_pos.y - 15
			};
			const ImVec2 player_duration_rect_bottom_right =
			{
				start_pos.x + mState.mTimelineWidth + 5,
				start_pos.y ,
			};

			bool draw_filled = false;
			if (mState.mIsWindowFocused)
			{
				if (mState.mAction->isAction<None>())
				{
					if (ImGui::IsMouseHoveringRect(player_duration_top_rect_left, player_duration_rect_bottom_right))
					{
						draw_filled = true;
						mState.mAction = createAction<HoveringSequenceDuration>();
					}
				}
				else if (mState.mAction->isAction<HoveringSequenceDuration>())
				{
					if (ImGui::IsMouseHoveringRect(player_duration_top_rect_left, player_duration_rect_bottom_right))
					{
						draw_filled = true;

						if (ImGui::IsMouseDown(0))
						{
							mState.mAction = createAction<DraggingSequenceDuration>();
						}

						if (ImGui::IsMouseDown(1))
						{
							mState.mAction = createAction<OpenSequenceDurationPopup>();
						}
					}
					else
					{
						mState.mAction = createAction<None>();
					}
				}
				else if (mState.mAction->isAction<DraggingSequenceDuration>())
				{
					if (ImGui::IsMouseDown(0))
					{
						draw_filled = true;
						float amount = mState.mMouseDelta.x / mState.mHorizontalResolution;
						double new_duration = player.getDuration() + amount;
						mEditor.changeSequenceDuration(new_duration);
					}
					else
					{
						mState.mAction = createAction<None>();
					}
					
				}
			}

			if (!draw_filled)
			{
				draw_list->AddRect(player_duration_top_rect_left, player_duration_rect_bottom_right, guicolors::white);
			}
			else
			{
				draw_list->AddRectFilled(player_duration_top_rect_left, player_duration_rect_bottom_right, guicolors::white);
			}
		}

		ImGui::EndChild();

		ImGui::PopID();
	}


	void SequenceEditorGUIView::drawTimelinePlayerPosition(
		const Sequence& sequence,
		SequencePlayer& player)
	{
		ImVec2 pos =
		{
			mState.mWindowPos.x + mState.mTimelineControllerPos.x - mState.mScroll.x
				+ mState.mInspectorWidth + 5
				+ mState.mTimelineWidth * (float)(player.getPlayerTime() / player.getDuration()) - 1,
				mState.mWindowPos.y + mState.mTimelineControllerPos.y + 15.0f - mState.mScroll.y
		};

		auto* overlay_drawlist = ImGui::GetOverlayDrawList();
		overlay_drawlist->AddLine({ pos.x, pos.y },
								  { pos.x, pos.y + sequence.mTracks.size() * (mState.mVerticalResolution + 10.0f ) + 10.0f },
								  guicolors::red, 2.0f);
	}



	void SequenceEditorGUIView::drawEndOfSequence(const Sequence& sequence, SequencePlayer& player)
	{
		std::ostringstream string_stream;
		string_stream << mID << "timelinesequenceduration";
		std::string id_string = string_stream.str();

		// store cursorpos
		ImVec2 cursor_pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(
		{
			mState.mTimelineControllerPos.x
			+ mState.mInspectorWidth + 5
			+ mState.mTimelineWidth ,
			mState.mTimelineControllerPos.y + 15.0f
		});

		ImGui::PushStyleColor(ImGuiCol_ChildBg, guicolors::white);
		if (ImGui::BeginChild(id_string.c_str(), // id
		{ 1.0f, sequence.mTracks.size() * (mState.mVerticalResolution + 10.0f) + 10.0f }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{


		}
		ImGui::EndChild();
		ImGui::PopStyleColor();

		// pop cursorpos
		ImGui::SetCursorPos(cursor_pos);
	}


	void SequenceEditorGUIView::handleLoadPopup()
	{
		if (mState.mAction->isAction<LoadPopup>())
		{
			auto* load_action = mState.mAction->getDerived<LoadPopup>();

			//
			if (ImGui::BeginPopupModal(
				"Load",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				const std::string show_dir = "sequences/";

				// Find all files in the preset directory
				std::vector<std::string> files_in_directory;
				utility::listDir(show_dir.c_str(), files_in_directory);

				std::vector<std::string> shows;
				std::vector<std::string> show_files;
				for (const auto& filename : files_in_directory)
				{
					// Ignore directories
					if (utility::dirExists(filename))
						continue;

					if (utility::getFileExtension(filename) == "json")
					{
						shows.emplace_back(utility::getFileName(filename));
						show_files.emplace_back(filename);
					}
				}

				SequenceTrackView::Combo("Sequences",
					&load_action->mSelectedShowIndex,
					shows);
					
				utility::ErrorState error_state;
				if (ImGui::Button("Load"))
				{
					if (mEditor.mSequencePlayer->load(utility::getFileName(show_files[load_action->mSelectedShowIndex]),
													  error_state))
					{
						mState.mAction = createAction<None>();
						mState.mDirty = true;
						ImGui::CloseCurrentPopup();
					}
					else
					{
						ImGui::OpenPopup("Error");
						load_action->mErrorString = error_state.toString();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text(load_action->mErrorString.c_str());
					if (ImGui::Button("OK"))
					{
						mState.mDirty = true;
						mState.mAction = createAction<LoadPopup>();
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::EndPopup();
			}
		}
	}


	void SequenceEditorGUIView::handleSaveAsPopup()
	{
		if (mState.mAction->isAction<SaveAsPopup>())
		{
			auto* save_as_action = mState.mAction->getDerived<SaveAsPopup>();

			// save as popup
			if (ImGui::BeginPopupModal(
				"Save As",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				const std::string show_dir = "sequences";

				// Find all files in the preset directory
				std::vector<std::string> files_in_directory;
				utility::listDir(show_dir.c_str(), files_in_directory);

				std::vector<std::string> shows;
				for (const auto& filename : files_in_directory)
				{
					// Ignore directories
					if (utility::dirExists(filename))
						continue;

					if (utility::getFileExtension(filename) == "json")
					{
						shows.push_back(utility::getFileName(filename));
					}
				}
				shows.push_back("<New...>");

				if (SequenceTrackView::Combo("Shows",
					&save_as_action->mSelectedShowIndex,
					shows))
				{
					if (save_as_action->mSelectedShowIndex == shows.size() - 1)
					{
						ImGui::OpenPopup("New");
					}
					else
					{
						ImGui::OpenPopup("Overwrite");
					}
				}

				// new show popup
				std::string new_show_filename;
				bool done = false;
				if (ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					static char name[256] = { 0 };
					ImGui::InputText("Name", name, 256);

					if (ImGui::Button("OK") && strlen(name) != 0)
					{
						new_show_filename = std::string(name, strlen(name));
						new_show_filename += ".json";

						ImGui::CloseCurrentPopup();
						done = true;
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel"))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
				if (done)
				{
					// Insert before the '<new...>' item
					shows.insert(shows.end() - 1, new_show_filename);

					utility::ErrorState error_state;
					
					if (mEditor.mSequencePlayer->save(utility::getFileName(new_show_filename), error_state))
					{
						save_as_action->mSelectedShowIndex = shows.size() - 2;
						mState.mDirty = true;
					}
					else
					{
						mState.mDirty = true;
						save_as_action->mErrorString = error_state.toString();
						ImGui::OpenPopup("Error");
					}
				}

				if (ImGui::BeginPopupModal("Overwrite"))
				{
					utility::ErrorState error_state;
					ImGui::Text(("Are you sure you want to overwrite " + 
						shows[save_as_action->mSelectedShowIndex] + " ?").c_str());
					if (ImGui::Button("OK"))
					{
						if (mEditor.mSequencePlayer->save(
							utility::getFileName(shows[save_as_action->mSelectedShowIndex]), error_state))
						{
						}
						else
						{
							save_as_action->mErrorString = error_state.toString();
							ImGui::OpenPopup("Error");
						}

						mState.mDirty = true;

						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				if (ImGui::BeginPopupModal("Error"))
				{
					ImGui::Text(save_as_action->mErrorString.c_str());
					if (ImGui::Button("OK"))
					{
						mState.mDirty = true;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Done"))
				{
					mState.mAction = createAction<None>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}


	void SequenceEditorGUIView::handleInsertTrackPopup()
	{
		if (mState.mAction->isAction<OpenInsertTrackPopup>())
		{
			mState.mAction = createAction<InsertingTrackPopup>();
			ImGui::OpenPopup("Insert New Track");
		}

		if (mState.mAction->isAction<InsertingTrackPopup>())
		{
			if (ImGui::BeginPopup("Insert New Track"))
			{
				for (auto& it : getTrackViewTypeViewMap())
				{
					const auto& name = it.first.get_name().to_string();
					if (ImGui::Button(name.c_str()))
					{
						auto* controller = mEditor.getControllerWithTrackType(it.first);
						assert(controller != nullptr);
						if (controller != nullptr)
						{
							controller->insertTrack(it.first);
							mState.mAction = createAction<None>();
							ImGui::CloseCurrentPopup();
						}
					}
				}

				if (ImGui::Button("Cancel"))
				{
					mState.mAction = createAction<None>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// clicked outside so exit popup
				mState.mAction = createAction<None>();
			}
		}
	}


	void SequenceEditorGUIView::handleSequenceDurationPopup()
	{
		if (mState.mAction->isAction<OpenSequenceDurationPopup>())
		{
			mState.mAction = createAction<EditSequenceDurationPopup>();
			ImGui::OpenPopup("Edit Sequence Duration");
		}

		if (mState.mAction->isAction<EditSequenceDurationPopup>())
		{
			if (ImGui::BeginPopup("Edit Sequence Duration"))
			{
				double duration = mEditor.mSequencePlayer->getDuration();

				int time_milseconds = (int)(duration * 100.0) % 100;
				int time_seconds = (int)(duration) % 60;
				int time_minutes = (int)(duration) / 60;

				bool edit_time = false;

				ImGui::Separator();

				ImGui::PushItemWidth(100.0f);

				int time_array[3] =
				{
					time_minutes,
					time_seconds,
					time_milseconds
				};

				edit_time = ImGui::InputInt3("Duration (mm:ss:ms)", &time_array[0]);
				time_array[0] = math::clamp<int>(time_array[0], 0, 99999);
				time_array[1] = math::clamp<int>(time_array[1], 0, 59);
				time_array[2] = math::clamp<int>(time_array[2], 0, 99);

				if (edit_time)
				{
					double new_duration = (((double)time_array[2]) / 100.0) + (double)time_array[1] + ((double)time_array[0] * 60.0);
					mEditor.changeSequenceDuration(new_duration);
					mState.mDirty = true;
				}

				ImGui::PopItemWidth();

				ImGui::Separator();

				if (ImGui::Button("Done"))
				{
					mState.mAction = createAction<None>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
			else
			{
				// clicked outside so exit popup
				mState.mAction = createAction<None>();
			}
		}
	}
}
