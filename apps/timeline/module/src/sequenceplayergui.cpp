// local includes
#include "sequenceplayergui.h"
#include "napcolors.h"
#include "sequencetracksegmentcurve.h"
#include "sequencetrack.h"

// External Includes
#include <entity.h>
#include <imgui/imgui.h>
#include <nap/logger.h>
#include <utility/fileutils.h>
#include <iomanip>

RTTI_BEGIN_CLASS(nap::SequencePlayerGUI)
RTTI_PROPERTY("Sequence Player", &nap::SequencePlayerGUI::mSequencePlayer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool SequencePlayerGUI::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		mView = std::make_unique<SequencePlayerGUIView>(
			*mSequencePlayer.get(),
			mID);

		return true;
	}


	void SequencePlayerGUI::onDestroy()
	{
	}


	void SequencePlayerGUI::draw()
	{
		//
		mView->draw();
	}


	SequencePlayerGUIView::SequencePlayerGUIView(SequencePlayer& player, std::string id)
		: mPlayer(player), mID(id)
	{
	}


	void SequencePlayerGUIView::draw()
	{
		//
		mInspectorWidth = 200.0f;

		//
		mMousePos = ImGui::GetMousePos();
		mMouseDelta = { mMousePos.x - mPreviousMousePos.x, mMousePos.y - mPreviousMousePos.y };
		mPreviousMousePos = mMousePos;

		//
		const Sequence& sequence = mPlayer.getSequenceConst();

		// push id
		ImGui::PushID(mID.c_str());

		// 100 px per second, default
		mStepSize = mHorizontalResolution;

		// calc width of content in timeline window
		mTimelineWidth =
			mStepSize * sequence.mDuration;


		// set content width of next window
		ImGui::SetNextWindowContentWidth(mTimelineWidth + mInspectorWidth + mVerticalResolution);

		// begin window
		if (ImGui::Begin(
			mID.c_str(), // id
			(bool*)0, // open
			ImGuiWindowFlags_HorizontalScrollbar)) // window flags
		{
			// we want to know if this window is focused in order to handle mouseinput
			// in child windows or not
			mIsWindowFocused = ImGui::IsRootWindowOrAnyChildFocused();

			// clear curve cache if we move the window
			mWindowPos = ImGui::GetWindowPos();
			if (mWindowPos.x != mPrevWindowPos.x ||
				mWindowPos.y != mPrevWindowPos.y)
			{
				mCurveCache.clear();
			}
			mPrevWindowPos = mWindowPos;

			// clear curve cache if we scroll inside the window
			ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
			if (scroll.x != mPrevScroll.x || scroll.y != mPrevScroll.y)
			{
				mCurveCache.clear();
			}
			mPrevScroll = scroll;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX());

			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				ImGui::OpenPopup("Load");
				mPlayerAction.currentAction = SequencePlayerActions::LOAD;
				mPlayerAction.currentActionData = std::make_unique<SequencePlayerGUILoadShowData>();
			}

			ImGui::SameLine();

			if (mPlayer.getIsPlaying())
			{
				if (ImGui::Button("Stop"))
				{
					mPlayer.stop();
				}
			}
			else
			{
				if (ImGui::Button("Play"))
				{
					mPlayer.play();
				}
			}

			ImGui::SameLine();
			if (mPlayer.getIsPaused() && mPlayer.getIsPlaying())
			{
				if (ImGui::Button("Play"))
				{
					mPlayer.play();
				}
			}
			else
			{
				if (ImGui::Button("Pause"))
				{
					mPlayer.pause();
				}

			}

			ImGui::SameLine();
			if (ImGui::Button("Rewind"))
			{
				mPlayer.setPlayerTime(0.0);
			}

			ImGui::SameLine();
			bool isLooping = mPlayer.getIsLooping();
			if (ImGui::Checkbox("Loop", &isLooping))
			{
				mPlayer.setIsLooping(isLooping);
			}

			ImGui::SameLine();
			float playbackSpeed = mPlayer.getPlaybackSpeed();
			ImGui::PushItemWidth(50.0f);
			if (ImGui::DragFloat("speed", &playbackSpeed, 0.01f, -10.0f, 10.0f, "%.1f"))
			{
				playbackSpeed = math::clamp(playbackSpeed, -10.0f, 10.0f);
				mPlayer.setPlaybackSpeed(playbackSpeed);
			}
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::PushItemWidth(200.0f);
			if (ImGui::DragFloat("H-Zoom", &mHorizontalResolution, 0.5f, 10, 1000, "%0.1f"))
				mCurveCache.clear();
			ImGui::SameLine();
			if (ImGui::DragFloat("V-Zoom", &mVerticalResolution, 0.5f, 100, 1000, "%0.1f"))
				mCurveCache.clear();
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// store position of next window ( player controller ), we need it later to draw the timelineplayer position 
			mTimelineControllerPos = ImGui::GetCursorPos();
			drawPlayerController(mPlayer);

			// move a little bit more up to align tracks nicely with timelinecontroller
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mVerticalResolution - 10);

			// draw tracks
			drawTracks(mPlayer, sequence);

			// on top of everything, draw time line player position
			drawTimelinePlayerPosition(sequence, mPlayer);

			//
			handleLoadPopup();
		}

		ImGui::End();

		// pop id
		ImGui::PopID();
	}


	void SequencePlayerGUIView::drawTracks(
		const SequencePlayer& sequencePlayer,
		const Sequence &sequence)
	{
		// get current cursor pos, we will use this to position the track windows
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// define consts
		mTrackHeight = mVerticalResolution;
		const float marginBetweenTracks = 10.0f;

		int trackCount = 0;
		for (const auto& track : sequence.mTracks)
		{
			// begin inspector
			std::ostringstream inspectorIDStream;
			inspectorIDStream << track->mID << "inspector";
			std::string inspectorID = inspectorIDStream.str();

			// manually set the cursor position before drawing new track window
			cursorPos =
			{
				cursorPos.x ,
				mTrackHeight + marginBetweenTracks + cursorPos.y
			};

			// manually set the cursor position before drawing inspector
			ImVec2 inspectorCursorPos = { cursorPos.x , cursorPos.y };
			ImGui::SetCursorPos(inspectorCursorPos);

			// draw inspector window
			if (ImGui::BeginChild(
				inspectorID.c_str(), // id
				{ mInspectorWidth , mTrackHeight + 5 }, // size
				false, // no border
				ImGuiWindowFlags_NoMove)) // window flags
			{
				// obtain drawlist
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				// store window size and position
				const ImVec2 windowPos = ImGui::GetWindowPos();
				const ImVec2 windowSize = ImGui::GetWindowSize();

				// draw background & box
				drawList->AddRectFilled(
					windowPos,
					{ windowPos.x + windowSize.x - 5, windowPos.y + mTrackHeight },
					guicolors::black);

				drawList->AddRect(
					windowPos,
					{ windowPos.x + windowSize.x - 5, windowPos.y + mTrackHeight },
					guicolors::white);

				// 
				ImVec2 inspectorCursorPos = ImGui::GetCursorPos();
				inspectorCursorPos.x += 5;
				inspectorCursorPos.y += 5;
				ImGui::SetCursorPos(inspectorCursorPos);

				// scale down everything
				float scale = 0.25f;
				ImGui::GetStyle().ScaleAllSizes(scale);

				// draw the assigned parameter
				ImGui::Text("Assigned Parameter");

				inspectorCursorPos = ImGui::GetCursorPos();
				inspectorCursorPos.x += 5;
				inspectorCursorPos.y += 5;
				ImGui::SetCursorPos(inspectorCursorPos);

				ImGui::PushItemWidth(200.0f);

				std::string name = track->mAssignedParameterID;
				if (name == "")
					name = "none";
				ImGui::Text(name.c_str());

				//
				ImGui::PopItemWidth();

				// pop scale
				ImGui::GetStyle().ScaleAllSizes(1.0f / scale);
			}
			ImGui::EndChild();

			const ImVec2 windowCursorPos = { cursorPos.x + mInspectorWidth + 5, cursorPos.y };
			ImGui::SetCursorPos(windowCursorPos);

			// begin track
			if (ImGui::BeginChild(
				track->mID.c_str(), // id
				{ mTimelineWidth + 5 , mTrackHeight + 5 }, // size
				false, // no border
				ImGuiWindowFlags_NoMove)) // window flags
			{
				// push id
				ImGui::PushID(track->mID.c_str());

				// get child focus
				bool trackHasFocus = ImGui::IsMouseHoveringWindow();

				// get window drawlist
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				// get current imgui cursor position
				ImVec2 cursorPos = ImGui::GetCursorPos();

				// get window position
				ImVec2 windowTopLeft = ImGui::GetWindowPos();

				// calc beginning of timeline graphic
				ImVec2 trackTopLeft = { windowTopLeft.x + cursorPos.x, windowTopLeft.y + cursorPos.y };

				// draw background of track
				drawList->AddRectFilled(
					trackTopLeft, // top left position
					{ trackTopLeft.x + mTimelineWidth, trackTopLeft.y + mTrackHeight }, // bottom right position
					guicolors::black); // color 

									   // draw border of track
				drawList->AddRect(
					trackTopLeft, // top left position
					{ trackTopLeft.x + mTimelineWidth, trackTopLeft.y + mTrackHeight }, // bottom right position
					guicolors::white); // color 

				float previousSegmentX = 0.0f;

				SequenceTrackTypes::Types trackType = track->getTrackType();

				int segmentCount = 0;
				for (const auto& segment : track->mSegments)
				{
					float segmentX = (segment->mStartTime + segment->mDuration) * mStepSize;
					float segmentWidth = segment->mDuration * mStepSize;

					if (trackType == SequenceTrackTypes::Types::FLOAT)
					{
						drawSegmentContent<float>(
							*track.get(),
							*segment.get(),
							trackTopLeft,
							previousSegmentX,
							segmentWidth,
							segmentX,
							drawList,
							(segmentCount == 0));
					}
					else if (trackType == SequenceTrackTypes::Types::VEC3)
					{
						drawSegmentContent<glm::vec3>(
							*track.get(),
							*segment.get(),
							trackTopLeft,
							previousSegmentX,
							segmentWidth,
							segmentX,
							drawList,
							(segmentCount == 0));
					}
					else if (trackType == SequenceTrackTypes::Types::VEC2)
					{
						drawSegmentContent<glm::vec2>(
							*track.get(),
							*segment.get(),
							trackTopLeft,
							previousSegmentX,
							segmentWidth,
							segmentX,
							drawList,
							(segmentCount == 0));
					}
					else if (trackType == SequenceTrackTypes::Types::VEC4)
					{
						drawSegmentContent<glm::vec4>(
							*track.get(),
							*segment.get(),
							trackTopLeft,
							previousSegmentX,
							segmentWidth,
							segmentX,
							drawList,
							(segmentCount == 0));
					}

					//
					previousSegmentX = segmentX;

					//
					segmentCount++;
				}

				// pop id
				ImGui::PopID();

			}

			ImGui::End();

			//
			ImGui::SetCursorPos(cursorPos);

			// increment track count
			trackCount++;
		}
	}


	void SequencePlayerGUIView::drawPlayerController(SequencePlayer& player)
	{
		const float timelineControllerHeight = 30.0f;

		std::ostringstream stringStream;
		stringStream << mID << "timelinecontroller";
		std::string idString = stringStream.str();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + mInspectorWidth + 5.0f);
		ImGui::PushID(idString.c_str());

		// used for culling ( is stuff inside the parent window ??? )
		ImVec2 parentWindowPos = ImGui::GetWindowPos();
		ImVec2 parentWindowSize = ImGui::GetWindowSize();

		// draw timeline controller
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ mTimelineWidth + 5 , timelineControllerHeight }, // size
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
					startPos.x + mTimelineWidth,
					startPos.y + timelineControllerHeight - 15
				}, guicolors::black);

			// draw box of controller
			drawList->AddRect(
				startPos,
				{
					startPos.x + mTimelineWidth,
					startPos.y + timelineControllerHeight - 15
				}, guicolors::white);

			// draw handler of player position
			const double playerTime = player.getPlayerTime();
			const ImVec2 playerTimeRectTopLeft =
			{
				startPos.x + (float)(playerTime / player.getDuration()) * mTimelineWidth - 5,
				startPos.y
			};
			const ImVec2 playerTimeRectBottomRight =
			{
				startPos.x + (float)(playerTime / player.getDuration()) * mTimelineWidth + 5,
				startPos.y + timelineControllerHeight,
			};

			drawList->AddRectFilled(
				playerTimeRectTopLeft,
				playerTimeRectBottomRight,
				guicolors::red);

			// draw timestamp text every 100 pixels
			const float timestampInterval = 100.0f;
			int steps = mTimelineWidth / timestampInterval;
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
						double timeInPlayer = mPlayer.getSequenceConst().mDuration * (float)((float)i / steps);
						std::string formattedTimeString = formatTimeString(timeInPlayer);
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

			if (mIsWindowFocused)
			{
				if (mPlayerAction.currentAction == SequencePlayerActions::NONE ||
					mPlayerAction.currentAction == SequencePlayerActions::HOVERING_PLAYER_TIME)
				{
					if (ImGui::IsMouseHoveringRect(startPos,
					{
						startPos.x + mTimelineWidth,
						startPos.y + timelineControllerHeight
					}))
					{
						mPlayerAction.currentAction = SequencePlayerActions::HOVERING_PLAYER_TIME;

						if (ImGui::IsMouseDown(0))
						{
							//
							bool playerWasPlaying = player.getIsPlaying();
							bool playerWasPaused = player.getIsPaused();

							mPlayerAction.currentAction = SequencePlayerActions::DRAGGING_PLAYER_TIME;
							mPlayerAction.currentActionData = std::make_unique<SequenceGUIPlayerDragPlayerData>(
								playerWasPlaying,
								playerWasPaused
								);

							if (playerWasPlaying)
							{
								player.pause();
							}

							// snap to mouse position
							double time = ((ImGui::GetMousePos().x - startPos.x) / mTimelineWidth) * player.getDuration();
							player.setPlayerTime(time);
						}
					}
					else
					{
						mPlayerAction.currentAction = SequencePlayerActions::NONE;
					}
				}
				else if (mPlayerAction.currentAction == SequencePlayerActions::DRAGGING_PLAYER_TIME)
				{
					if (ImGui::IsMouseDown(0))
					{
						double delta = (mMouseDelta.x / mTimelineWidth) * player.getDuration();
						player.setPlayerTime(playerTime + delta);
					}
					else
					{
						if (ImGui::IsMouseReleased(0))
						{
							const SequenceGUIPlayerDragPlayerData* data = 
								dynamic_cast<SequenceGUIPlayerDragPlayerData*>(mPlayerAction.currentActionData.get());
							if (data->playerWasPlaying && !data->playerWasPaused)
							{
								player.play();
							}

							mPlayerAction.currentAction = SequencePlayerActions::NONE;
						}
					}
				}
			}
		}

		ImGui::EndChild();

		ImGui::PopID();
	}


	void SequencePlayerGUIView::drawTimelinePlayerPosition(
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
			mTimelineControllerPos.x
			+ mInspectorWidth + 5
			+ mTimelineWidth * (float)(player.getPlayerTime() / player.getDuration()) - 1,
			mTimelineControllerPos.y + 15.0f
		});

		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, guicolors::red);
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ 1.0f, sequence.mTracks.size() * (mVerticalResolution + 10.0f) + 10.0f }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{


		}
		ImGui::End();
		ImGui::PopStyleColor();

		// pop cursorpos
		ImGui::SetCursorPos(cursorPos);
	}


	void SequencePlayerGUIView::handleLoadPopup()
	{
		if (mPlayerAction.currentAction == SequencePlayerActions::LOAD)
		{
			//
			if (ImGui::BeginPopupModal(
				"Load",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				SequencePlayerGUILoadShowData* data = dynamic_cast<SequencePlayerGUILoadShowData*>(mPlayerAction.currentActionData.get());
				assert(data != nullptr);

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
				Combo("Sequences",
					&data->selectedShow,
					shows);

				utility::ErrorState errorState;
				if (ImGui::Button("Load"))
				{
					if (mPlayer.load(
						showFiles[data->selectedShow], errorState))
					{
						mPlayerAction.currentAction = SequencePlayerActions::NONE;
						mPlayerAction.currentActionData = nullptr;
						mCurveCache.clear();
						ImGui::CloseCurrentPopup();
					}
					else
					{
						ImGui::OpenPopup("Error");
						data->errorString = errorState.toString();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					mPlayerAction.currentAction = SequencePlayerActions::NONE;
					mPlayerAction.currentActionData = nullptr;
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::BeginPopupModal(
					"Error",
					nullptr, 
					ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text(data->errorString.c_str());
					if (ImGui::Button("OK"))
					{
						mPlayerAction.currentAction = SequencePlayerActions::LOAD;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::EndPopup();
			}
		}
	}


	template<typename T>
	void SequencePlayerGUIView::drawCurves(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float previousSegmentX,
		const float segmentWidth,
		const float segmentX,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment = 
			segmentBase.getDerivedConst<SequenceTrackSegmentCurve<T>>();

		const int resolution = 40;
		bool curveSelected = false;

		if (mCurveCache.find(segment.mID) == mCurveCache.end())
		{
			std::vector<ImVec2> points;
			points.resize((resolution + 1)*segment.mCurves.size());
			for (int v = 0; v < segment.mCurves.size(); v++)
			{
				for (int i = 0; i <= resolution; i++)
				{
					float value = 1.0f - segment.mCurves[v]->evaluate((float)i / resolution);

					points[i + v * (resolution + 1)] =
					{
						trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / resolution),
						trackTopLeft.y + value * mTrackHeight
					};
				}
			}
			mCurveCache.emplace(segment.mID, points);
		}

		for (int i = 0; i < segment.mCurves.size(); i++)
		{
			// draw points of curve
			drawList->AddPolyline(
				&*mCurveCache[segment.mID].begin() + i * (resolution + 1), // points array
				mCurveCache[segment.mID].size() / segment.mCurves.size(), // size of points array
				guicolors::curvecolors[i], // color
				false, // closed
				1.0f, // thickness
				true); // anti-aliased
		}
	}

	template<typename T>
	void SequencePlayerGUIView::drawSegmentContent(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		const ImVec2& trackTopLeft,
		float previousSegmentX,
		float segmentWidth,
		float segmentX,
		ImDrawList* drawList,
		bool drawStartValue)
	{
		// curves
		drawCurves<T>(
			track,
			segment,
			trackTopLeft,
			previousSegmentX,
			segmentWidth,
			segmentX,
			drawList);
	}


	static bool vector_getter(void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool SequencePlayerGUIView::Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	bool SequencePlayerGUIView::ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	std::string SequencePlayerGUIView::formatTimeString(double time)
	{
		int hours = time / 3600.0f;
		int minutes = (int)(time / 60.0f) % 60;
		int seconds = (int)time % 60;
		int milliseconds = (int)(time * 100.0f) % 100;

		std::stringstream stringStream;

		stringStream << std::setw(2) << std::setfill('0') << seconds;
		std::string secondsString = stringStream.str();

		stringStream = std::stringstream();
		stringStream << std::setw(2) << std::setfill('0') << minutes;
		std::string minutesString = stringStream.str();

		stringStream = std::stringstream();
		stringStream << std::setw(2) << std::setfill('0') << milliseconds;
		std::string millisecondsStrings = stringStream.str();

		std::string hoursString = "";
		if (hours > 0)
		{
			stringStream = std::stringstream();
			stringStream << std::setw(2) << std::setfill('0') << hours;
			hoursString = stringStream.str() + ":";
		}

		return hoursString + minutesString + ":" + secondsString + ":" + millisecondsStrings;
	}
}