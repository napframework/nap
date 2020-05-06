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
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

using namespace nap::SequenceGUIActions;
using namespace nap::SequenceEditorTypes;

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

		mView = std::make_unique<SequenceEditorGUIView>(*mSequenceEditor.get(), mID);
		
		return true;
	}


	void SequenceEditorGUI::onDestroy()
	{
	}


	void SequenceEditorGUI::show()
	{
		mView->show();
	}


	SequenceEditorGUIView::SequenceEditorGUIView(SequenceEditor& editor, std::string id)
		: mEditor(editor), mID(id)
	{
		for (auto& factory : SequenceTrackView::getFactoryMap())
		{
			mViews.emplace(factory.first, factory.second(*this));
		}

		mState.mAction = createAction<None>();
	}


	void SequenceEditorGUIView::show()
	{
		//
		mState.mInspectorWidth = 300.0f;

		//
		mState.mMousePos = ImGui::GetMousePos();
		mState.mMouseDelta = 
		{	mState.mMousePos.x - mState.mPreviousMousePos.x, 
			mState.mMousePos.y - mState.mPreviousMousePos.y };
		mState.mPreviousMousePos = mState.mMousePos;

		//
		const Sequence& sequence = mEditor.mSequencePlayer->getSequence();
		SequencePlayer& sequencePlayer = *mEditor.mSequencePlayer.get();

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		mState.mStepSize = mState.mHorizontalResolution;

		// calc width of content in timeline window
		mState.mTimelineWidth = mState.mStepSize * sequence.mDuration;

		// set content width of next window
		ImGui::SetNextWindowContentWidth(mState.mTimelineWidth + mState.mInspectorWidth + mState.mVerticalResolution);

		// begin window
		if (ImGui::Begin(
			mID.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{
			// we want to know if this window is focused in order to handle mouseinput
			// in child windows or not
			mState.mIsWindowFocused = ImGui::IsRootWindowOrAnyChildFocused();

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
			if (scroll.x != mState.mPrevScroll.x || scroll.y != mState.mPrevScroll.y)
			{
				mState.mDirty = true;
			}
			mState.mPrevScroll = scroll;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetScrollX());

			//
			if (ImGui::Button("Save"))
			{
				//mController.save();
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

			if (sequencePlayer.getIsPlaying())
			{
				if (ImGui::Button("Stop"))
				{
					sequencePlayer.setIsPlaying(false);
				}
			}
			else
			{
				if (ImGui::Button("Play"))
				{
					sequencePlayer.setIsPlaying(true);
				}
			}

			ImGui::SameLine();
			if (sequencePlayer.getIsPaused())
			{
				if (ImGui::Button("Unpause"))
				{
					sequencePlayer.setIsPaused(false);
				}
			}
			else
			{
				if (ImGui::Button("Pause"))
				{
					sequencePlayer.setIsPaused(true);
				}
			}
			

			ImGui::SameLine();
			if (ImGui::Button("Rewind"))
			{
				sequencePlayer.setPlayerTime(0.0);
			}

			ImGui::SameLine();
			bool isLooping = sequencePlayer.getIsLooping();
			if (ImGui::Checkbox("Loop", &isLooping))
			{
				sequencePlayer.setIsLooping(isLooping);
			}

			ImGui::SameLine();
			float playbackSpeed = sequencePlayer.getPlaybackSpeed();
			ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("speed", &playbackSpeed, 0.01f, -10.0f, 10.0f, "%.1f"))
			{
				playbackSpeed = math::clamp(playbackSpeed, -10.0f, 10.0f);
				sequencePlayer.setPlaybackSpeed(playbackSpeed);
			}
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetScrollX());

			ImGui::PushItemWidth(200.0f);
			if (ImGui::DragFloat("H-Zoom", &mState.mHorizontalResolution, 0.5f, 10, 1000, "%0.1f"))
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
			drawPlayerController(sequencePlayer);

			// move a little bit more up to align tracks nicely with timelinecontroller
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mState.mVerticalResolution - 10);

			// draw tracks
			drawTracks(sequencePlayer, sequence);
				
			// on top of everything, draw time line player position
			drawTimelinePlayerPosition(sequence, sequencePlayer);

			// move the cursor below the tracks
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mState.mVerticalResolution + 10.0f);
			if (ImGui::Button("Insert New Track"))
			{
				mState.mAction = createAction<OpenInsertTrackPopup>();
			}

			// handle popups
			for (auto& it : mViews)
			{
				it.second->handlePopups(mState);
			}

			//
			handleInsertTrackPopup();

			//
			handleLoadPopup();

			//
			handleSaveAsPopup();
		}

		ImGui::End();

		// pop id
		ImGui::PopID();

		//
		mState.mDirty = false;
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
					it2->second->drawTrack(*sequence.mTracks[i].get(), mState);
				}
			}
		}
	}


	void SequenceEditorGUIView::drawPlayerController(SequencePlayer& player)
	{
		const float timelineControllerHeight = 30.0f;

		std::ostringstream stringStream;
		stringStream << mID << "timelinecontroller";
		std::string idString = stringStream.str();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + mState.mInspectorWidth + 5.0f);
		ImGui::PushID(idString.c_str());

		// used for culling ( is stuff inside the parent window ??? )
		ImVec2 parentWindowPos = ImGui::GetWindowPos();
		ImVec2 parentWindowSize = ImGui::GetWindowSize();

		// draw timeline controller
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ mState.mTimelineWidth + 5 , timelineControllerHeight }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			ImVec2 cursorPos = ImGui::GetCursorPos();
			ImVec2 windowTopLeft = ImGui::GetWindowPos();
			ImVec2 startPos =
			{
				windowTopLeft.x + cursorPos.x,
				windowTopLeft.y + cursorPos.y + 15,
			};

			cursorPos.y += 5;

			// get window drawlist
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// draw backgroundbox of controller
			drawList->AddRectFilled(
				startPos,
				{
					startPos.x + mState.mTimelineWidth,
					startPos.y + timelineControllerHeight - 15
				}, guicolors::black);

			// draw box of controller
			drawList->AddRect(
				startPos,
				{
					startPos.x + mState.mTimelineWidth,
					startPos.y + timelineControllerHeight - 15
				}, guicolors::white);

			// draw handler of player position
			const double playerTime = player.getPlayerTime();
			const ImVec2 playerTimeRectTopLeft =
			{
				startPos.x + (float)(playerTime / player.getDuration()) * mState.mTimelineWidth - 5,
				startPos.y
			};
			const ImVec2 playerTimeRectBottomRight =
			{
				startPos.x + (float)(playerTime / player.getDuration()) * mState.mTimelineWidth + 5,
				startPos.y + timelineControllerHeight,
			};

			drawList->AddRectFilled(
				playerTimeRectTopLeft,
				playerTimeRectBottomRight,
				guicolors::red);

			// draw timestamp text every 100 pixels
			const float timestampInterval = 100.0f;
			int steps = mState.mTimelineWidth / timestampInterval;
			for (int i = 0; i < steps; i++)
			{
				ImVec2 timestampPos;
				timestampPos.x = i * timestampInterval + startPos.x;
				timestampPos.y = startPos.y - 18;

				if (timestampPos.x < parentWindowSize.x + parentWindowPos.x &&
					timestampPos.x >= parentWindowPos.x)
				{
					if (timestampPos.y >= parentWindowPos.y &&
						timestampPos.y < parentWindowSize.y + parentWindowPos.y)
					{
						double timeInPlayer = 0.0;// mPlayer.mDuration * (float)((float)i / steps);
						std::string formattedTimeString = SequenceTrackView::formatTimeString(timeInPlayer);
						drawList->AddText(timestampPos, guicolors::white, formattedTimeString.c_str());

						if (i != 0)
						{
							drawList->AddLine(
							{ timestampPos.x, timestampPos.y + 18 },
							{ timestampPos.x, timestampPos.y + timelineControllerHeight + 2 }, guicolors::darkGrey);
						}
					}
				}
			}

			if (mState.mIsWindowFocused)
			{
				if (mState.mAction->isAction<None>()|| mState.mAction->isAction<HoveringPlayerTime>())
				{
					if (ImGui::IsMouseHoveringRect(startPos, 
					{
						startPos.x + mState.mTimelineWidth,
						startPos.y + timelineControllerHeight
					}))
					{
						mState.mAction = createAction<HoveringPlayerTime>();

						if (ImGui::IsMouseDown(0))
						{
							//
							bool playerWasPlaying = player.getIsPlaying();
							bool playerWasPaused = player.getIsPaused();

							mState.mAction = createAction<DraggingPlayerTime>(playerWasPlaying, playerWasPaused);
							if (playerWasPlaying)
							{
								player.setIsPaused(true);
							}
							
							// snap to mouse position
							double time = ((ImGui::GetMousePos().x - startPos.x) / mState.mTimelineWidth) * player.getDuration();
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
						player.setPlayerTime(playerTime + delta);
					}
					else
					{
						if (ImGui::IsMouseReleased(0))
						{
							const auto* dragAction = mState.mAction->getDerived<DraggingPlayerTime>();
							assert(dragAction != nullptr); 
							if (dragAction->mWasPlaying && !dragAction->mWasPaused)
							{
								player.setIsPlaying(true);
							}

							mState.mAction = createAction<None>();
						}
					}
				}
			}
		}

		ImGui::EndChild();

		ImGui::PopID();
	}


	void SequenceEditorGUIView::drawTimelinePlayerPosition(
		const Sequence& sequence,
		SequencePlayer& player)
	{
		std::ostringstream stringStream;
		stringStream << mID << "timelineplayerposition";
		std::string idString = stringStream.str();

		// store cursorpos
		ImVec2 cursorPos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(
		{
			mState.mTimelineControllerPos.x
				+ mState.mInspectorWidth + 5
				+ mState.mTimelineWidth * (float)(player.getPlayerTime() / player.getDuration()) - 1,
			mState.mTimelineControllerPos.y + 15.0f
		});

		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, guicolors::red);
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ 1.0f, sequence.mTracks.size() * (mState.mVerticalResolution + 10.0f ) + 10.0f }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{

			
		}
		ImGui::End();
		ImGui::PopStyleColor();

		// pop cursorpos
		ImGui::SetCursorPos(cursorPos);
	}


	void SequenceEditorGUIView::handleLoadPopup()
	{
		if (mState.mAction->isAction<LoadPopup>())
		{
			auto* loadAction = mState.mAction->getDerived<LoadPopup>();

			//
			if (ImGui::BeginPopupModal(
				"Load",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				const std::string showDir = "sequences/";

				// Find all files in the preset directory
				std::vector<std::string> files_in_directory;
				utility::listDir(showDir.c_str(), files_in_directory);

				std::vector<std::string> shows;
				std::vector<std::string> showFiles;
				for (const auto& filename : files_in_directory)
				{
					// Ignore directories
					if (utility::dirExists(filename))
						continue;

					if (utility::getFileExtension(filename) == "json")
					{
						shows.emplace_back(utility::getFileName(filename));
						showFiles.emplace_back(filename);
					}
				}

				int index = 0;
				SequenceTrackView::Combo("Sequences",
					&loadAction->mSelectedShowIndex,
					shows);
					
				utility::ErrorState errorState;
				if (ImGui::Button("Load"))
				{
					
					if (mEditor.mSequencePlayer->load(
						showFiles[loadAction->mSelectedShowIndex], errorState))
					{
						mState.mAction = createAction<None>();
						mState.mDirty = true;
						ImGui::CloseCurrentPopup();
					}
					else
					{
						ImGui::OpenPopup("Error");
						loadAction->mErrorString = errorState.toString();
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
					ImGui::Text(loadAction->mErrorString.c_str());
					if (ImGui::Button("OK"))
					{
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
			auto* saveAsAction = mState.mAction->getDerived<SaveAsPopup>();

			// save as popup
			if (ImGui::BeginPopupModal(
				"Save As",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				const std::string showDir = "sequences";

				// Find all files in the preset directory
				std::vector<std::string> files_in_directory;
				utility::listDir(showDir.c_str(), files_in_directory);

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
					&saveAsAction->mSelectedShowIndex,
					shows))
				{
					if (saveAsAction->mSelectedShowIndex == shows.size() - 1)
					{
						ImGui::OpenPopup("New");
					}
					else
					{
						ImGui::OpenPopup("Overwrite");
					}
				}

				// new show popup
				std::string newShowFileName;
				bool done = false;
				if (ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					static char name[256] = { 0 };
					ImGui::InputText("Name", name, 256);

					if (ImGui::Button("OK") && strlen(name) != 0)
					{
						newShowFileName = std::string(name, strlen(name));
						newShowFileName += ".json";

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
					shows.insert(shows.end() - 1, newShowFileName);

					utility::ErrorState errorState;
					
					if (mEditor.mSequencePlayer->save(showDir + "/" + newShowFileName, errorState))
					{
						saveAsAction->mSelectedShowIndex = shows.size() - 2;
					}
					else
					{
						saveAsAction->mErrorString = errorState.toString();
						ImGui::OpenPopup("Error");
					}
				}

				if (ImGui::BeginPopupModal("Overwrite"))
				{
					utility::ErrorState errorState;
					ImGui::Text(("Are you sure you want to overwrite " + 
						shows[saveAsAction->mSelectedShowIndex] + " ?").c_str());
					if (ImGui::Button("OK"))
					{
						if (mEditor.mSequencePlayer->save(
							shows[saveAsAction->mSelectedShowIndex],
							errorState))
						{
						}
						else
						{
							saveAsAction->mErrorString = errorState.toString();
							ImGui::OpenPopup("Error");
						}

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
					ImGui::Text(saveAsAction->mErrorString.c_str());
					if (ImGui::Button("OK"))
					{
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
}
