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

using namespace nap::SequenceGUIMouseActions;
using namespace nap::SequenceEditorTypes;

namespace nap
{
	bool SequenceEditorGUI::init(utility::ErrorState& errorState)
	{
		if (!Resource::init(errorState))
		{
			return false;
		}

		mView = std::make_unique<SequenceEditorGUIView>(
			mSequenceEditor->getController(),
			mID);

		return true;
	}


	void SequenceEditorGUI::onDestroy()
	{
	}


	void SequenceEditorGUI::draw()
	{
		//
		mView->draw();
	}


	SequenceEditorGUIView::SequenceEditorGUIView(SequenceEditorController& controller, std::string id)
		: mController(controller), mID(id)
	{
	}


	void SequenceEditorGUIView::draw()
	{
		//
		mInspectorWidth = 300.0f;

		//
		mMousePos = ImGui::GetMousePos();
		mMouseDelta = { mMousePos.x - mPreviousMousePos.x, mMousePos.y - mPreviousMousePos.y };
		mPreviousMousePos = mMousePos;

		//
		const Sequence& sequence = mController.getSequence();
		SequencePlayer& sequencePlayer = mController.getSequencePlayer();

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

			//
			if (ImGui::Button("Save"))
			{
				mController.save();
			}

			ImGui::SameLine();

			if (ImGui::Button("Save As"))
			{
				ImGui::OpenPopup("Save As");
				mEditorAction.currentAction = SAVE_AS;
				mEditorAction.currentActionData = std::make_unique<SequenceGUISaveShowData>();
			}

			ImGui::SameLine();

			if (ImGui::Button("Load"))
			{
				ImGui::OpenPopup("Load");
				mEditorAction.currentAction = LOAD;
				mEditorAction.currentActionData = std::make_unique<SequenceGUILoadShowData>();
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
			if (sequencePlayer.getIsPaused() && sequencePlayer.getIsPlaying())
			{
				if (ImGui::Button("Play"))
				{
					sequencePlayer.setIsPlaying(true);
				}
			}
			else
			{
				if (ImGui::Button("Pause"))
				{
					sequencePlayer.pause();
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

			ImGui::PushItemWidth(200.0f);
			if (ImGui::DragFloat("H-Zoom", &mHorizontalResolution, 0.5f, 10, 1000, "%0.1f"))
				mCurveCache.clear();
			ImGui::SameLine();
			if (ImGui::DragFloat("V-Zoom", &mVerticalResolution, 0.5f, 150, 1000, "%0.1f"))
				mCurveCache.clear();
			ImGui::PopItemWidth();

			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			
			// store position of next window ( player controller ), we need it later to draw the timelineplayer position 
			mTimelineControllerPos = ImGui::GetCursorPos();
			drawPlayerController(sequencePlayer);

			// move a little bit more up to align tracks nicely with timelinecontroller
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mVerticalResolution - 10);

			// draw tracks
			drawTracks(sequencePlayer, sequence);
				
			// on top of everything, draw time line player position
			drawTimelinePlayerPosition(sequence, sequencePlayer);
			
			// handle insert segment popup
			handleInsertSegmentPopup();

			// handle delete segment popup
			handleDeleteSegmentPopup();

			// handler insert track popup
			handleInsertTrackPopup();

			handleInsertEventSegmentPopup();

			// move the cursor below the tracks
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + mVerticalResolution + 10.0f);
			if (ImGui::Button("Insert New Track"))
			{
				ImGui::OpenPopup("Insert New Track");
				mEditorAction.currentAction = SequenceGUIMouseActions::OPEN_INSERT_TRACK_POPUP;
			}

			//
			handleLoadPopup();

			//
			handleSaveAsPopup();
		}

		ImGui::End();

		// pop id
		ImGui::PopID();
	}


	void SequenceEditorGUIView::drawTracks(
		const SequencePlayer& sequencePlayer,
		const Sequence &sequence)
	{
		//
		bool deleteTrack = false;
		std::string deleteTrackID = "";

		// get current cursor pos, we will use this to position the track windows
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// define consts
		mTrackHeight = mVerticalResolution;
		const float marginBetweenTracks = 10.0f;

		int trackCount = 0;
		for (const auto& track : sequence.mTracks)
		{
			switch (track->getTrackType())
			{
			case SequenceTrackTypes::FLOAT:
				drawCurveTrack<float>(
					*track.get(),
					cursorPos,
					marginBetweenTracks,
					sequencePlayer,
					deleteTrack,
					deleteTrackID);
				break;
			case SequenceTrackTypes::VEC2:
				drawCurveTrack<glm::vec2>(
					*track.get(),
					cursorPos,
					marginBetweenTracks,
					sequencePlayer,
					deleteTrack,
					deleteTrackID);
				break;;
			case SequenceTrackTypes::VEC3:
				drawCurveTrack<glm::vec3>(
					*track.get(),
					cursorPos,
					marginBetweenTracks,
					sequencePlayer,
					deleteTrack,
					deleteTrackID);
				break;
			case SequenceTrackTypes::VEC4:
				drawCurveTrack<glm::vec4>(
					*track.get(),
					cursorPos,
					marginBetweenTracks,
					sequencePlayer,
					deleteTrack,
					deleteTrackID);
				break;
			case SequenceTrackTypes::EVENT:
				drawEventTrack(
					*track.get(),
					cursorPos,
					marginBetweenTracks,
					sequencePlayer,
					deleteTrack,
					deleteTrackID);
				break;
			default:
				break;
			}

			// increment track count
			trackCount++;
		}

		// delete the track if we did a delete track action
		if (deleteTrack)
		{
			mController.deleteTrack(deleteTrackID);
			mCurveCache.clear();
		}
	}


	void SequenceEditorGUIView::drawEventTrack(
		const SequenceTrack &track,
		ImVec2 &cursorPos,
		const float marginBetweenTracks,
		const SequencePlayer &sequencePlayer,
		bool &deleteTrack,
		std::string &deleteTrackID)
	{
		// begin inspector
		std::ostringstream inspectorIDStream;
		inspectorIDStream << track.mID << "inspector";
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

			// draw the assigned receiver
			ImGui::Text("Assigned Receivers");

			inspectorCursorPos = ImGui::GetCursorPos();
			inspectorCursorPos.x += 5;
			inspectorCursorPos.y += 5;
			ImGui::SetCursorPos(inspectorCursorPos);

			bool assigned = false;
			std::string assignedID;
			std::vector<std::string> dispatcherIDs;
			int currentItem = 0;
			dispatcherIDs.emplace_back("none");
			int count = 0;
			const SequenceEventReceiver* assignedParameterPtr = nullptr;
			for (const auto& dispatcher : sequencePlayer.mEventReceivers)
			{
				count++;

				if (dispatcher->mID == track.mAssignedObjectIDs)
				{
					assigned = true;
					assignedID = dispatcher->mID;
					currentItem = count;
					assignedParameterPtr = dispatcher.get();
				}

				dispatcherIDs.emplace_back(dispatcher->mID);
			}

			ImGui::PushItemWidth(140.0f);
			if (Combo(
				"",
				&currentItem,
				dispatcherIDs))
			{
				if (currentItem != 0)
					mController.assignNewObjectID(track.mID, dispatcherIDs[currentItem]);
				else
					mController.assignNewObjectID(track.mID, "");

			}

			//
			ImGui::PopItemWidth();


			// delete track button
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

			// when we delete a track, we don't immediately call the controller because we are iterating track atm
			if (ImGui::SmallButton("Delete"))
			{
				deleteTrack = true;
				deleteTrackID = track.mID;
			}

			// pop scale
			ImGui::GetStyle().ScaleAllSizes(1.0f / scale);
		}
		ImGui::EndChild();

		const ImVec2 windowCursorPos = { cursorPos.x + mInspectorWidth + 5, cursorPos.y };
		ImGui::SetCursorPos(windowCursorPos);

		// begin track
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mTimelineWidth + 5 , mTrackHeight + 5 }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			// push id
			ImGui::PushID(track.mID.c_str());

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

								   //
			mMouseCursorTime = (mMousePos.x - trackTopLeft.x) / mStepSize;

			if (mIsWindowFocused)
			{
				// handle insertion of segment
				if (mEditorAction.currentAction == SequenceGUIMouseActions::NONE)
				{
					if (ImGui::IsMouseHoveringRect(
						trackTopLeft, // top left position
						{ trackTopLeft.x + mTimelineWidth, trackTopLeft.y + mTrackHeight }))
					{
						// position of mouse in track
						drawList->AddLine(
						{ mMousePos.x, trackTopLeft.y }, // top left
						{ mMousePos.x, trackTopLeft.y + mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness

								   // right mouse down
						if (ImGui::IsMouseClicked(1))
						{
							double time = mMouseCursorTime;

							//
							mEditorAction.currentAction = OPEN_INSERT_SEGMENT_POPUP;
							mEditorAction.currentActionData = std::make_unique<SequenceGUIInsertSegmentData>(track.mID, time, static_cast<SequenceTrackTypes::Types>(track.getTrackType()));
						}
					}
				}

				// draw line in track while in inserting segment popup
				if (mEditorAction.currentAction == SequenceGUIMouseActions::OPEN_INSERT_SEGMENT_POPUP || mEditorAction.currentAction == INSERTING_SEGMENT)
				{
					const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mEditorAction.currentActionData.get());
					if (data->trackID == track.mID)
					{
						// position of insertion in track
						drawList->AddLine(
						{ trackTopLeft.x + (float)data->time * mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)data->time * mStepSize, trackTopLeft.y + mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness
					}
				}
			}

			float previousSegmentX = 0.0f;

			SequenceTrackTypes::Types trackType = track.getTrackType();

			int segmentCount = 0;
			for (const auto& segment : track.mSegments)
			{
				float segmentX = (segment->mStartTime) * mStepSize;
				float segmentWidth = segment->mDuration * mStepSize;

				// draw segment handlers
				drawSegmentHandler(
					track,
					*segment.get(),
					trackTopLeft,
					segmentX,
					0.0f,
					drawList);

				assert(segment->get_type().is_derived_from(RTTI_OF(SequenceTrackSegmentEvent)));
				const auto& segmentEvent = static_cast<const SequenceTrackSegmentEvent&>(*segment.get());

				drawList->AddText(
				{ trackTopLeft.x + segmentX + 5, trackTopLeft.y + 5 },
					guicolors::red,
					segmentEvent.mMessage.c_str());

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
	}

	template<typename T>
	void SequenceEditorGUIView::drawCurveTrack(
		const SequenceTrack &track,
		ImVec2 &cursorPos,
		const float marginBetweenTracks,
		const SequencePlayer &sequencePlayer,
		bool &deleteTrack,
		std::string &deleteTrackID)
	{
		// begin inspector
		std::ostringstream inspectorIDStream;
		inspectorIDStream << track.mID << "inspector";
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

			bool assigned = false;
			std::string assignedID;
			std::vector<std::string> parameterIDs;
			int currentItem = 0;
			parameterIDs.emplace_back("none");
			int count = 0;
			const Parameter* assignedParameterPtr = nullptr;
			for (const auto& parameter : sequencePlayer.mParameters)
			{
				count++;

				if (parameter->mID == track.mAssignedObjectIDs)
				{
					assigned = true;
					assignedID = parameter->mID;
					currentItem = count;
					assignedParameterPtr = parameter.get();
				}

				parameterIDs.emplace_back(parameter->mID);
			}

			ImGui::PushItemWidth(140.0f);
			if (Combo(
				"",
				&currentItem,
				parameterIDs))
			{
				if (currentItem != 0)
					mController.assignNewObjectID(track.mID, parameterIDs[currentItem]);
				else
					mController.assignNewObjectID(track.mID, "");

			}

			//
			ImGui::PopItemWidth();

			//
			drawInspectorRange<T>(track);

			// delete track button
			ImGui::Spacing();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

			// when we delete a track, we don't immediately call the controller because we are iterating track atm
			if (ImGui::SmallButton("Delete"))
			{
				deleteTrack = true;
				deleteTrackID = track.mID;
			}

			// pop scale
			ImGui::GetStyle().ScaleAllSizes(1.0f / scale);
		}
		ImGui::EndChild();

		const ImVec2 windowCursorPos = { cursorPos.x + mInspectorWidth + 5, cursorPos.y };
		ImGui::SetCursorPos(windowCursorPos);

		// begin track
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mTimelineWidth + 5 , mTrackHeight + 5 }, // size
			false, // no border
			ImGuiWindowFlags_NoMove)) // window flags
		{
			// push id
			ImGui::PushID(track.mID.c_str());

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

								   //
			mMouseCursorTime = (mMousePos.x - trackTopLeft.x) / mStepSize;

			if (mIsWindowFocused)
			{
				// handle insertion of segment
				if (mEditorAction.currentAction == SequenceGUIMouseActions::NONE)
				{
					if (ImGui::IsMouseHoveringRect(
						trackTopLeft, // top left position
						{ trackTopLeft.x + mTimelineWidth, trackTopLeft.y + mTrackHeight }))
					{
						// position of mouse in track
						drawList->AddLine(
						{ mMousePos.x, trackTopLeft.y }, // top left
						{ mMousePos.x, trackTopLeft.y + mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness

								   // right mouse down
						if (ImGui::IsMouseClicked(1))
						{
							double time = mMouseCursorTime;

							//
							mEditorAction.currentAction = OPEN_INSERT_SEGMENT_POPUP;
							mEditorAction.currentActionData = std::make_unique<SequenceGUIInsertSegmentData>(track.mID, time, static_cast<SequenceTrackTypes::Types>(track.getTrackType()));
						}
					}
				}

				// draw line in track while in inserting segment popup
				if (mEditorAction.currentAction == SequenceGUIMouseActions::OPEN_INSERT_SEGMENT_POPUP || mEditorAction.currentAction == INSERTING_SEGMENT)
				{
					const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mEditorAction.currentActionData.get());
					if (data->trackID == track.mID)
					{
						// position of insertion in track
						drawList->AddLine(
						{ trackTopLeft.x + (float)data->time * mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)data->time * mStepSize, trackTopLeft.y + mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness
					}
				}
			}

			float previousSegmentX = 0.0f;

			SequenceTrackTypes::Types trackType = track.getTrackType();

			int segmentCount = 0;
			for (const auto& segment : track.mSegments)
			{
				float segmentX = (segment->mStartTime + segment->mDuration) * mStepSize;
				float segmentWidth = segment->mDuration * mStepSize;

				if (trackType == SequenceTrackTypes::Types::FLOAT)
				{
					drawSegmentContent<float>(
						track,
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
						track,
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
						track,
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
						track,
						*segment.get(),
						trackTopLeft,
						previousSegmentX,
						segmentWidth,
						segmentX,
						drawList,
						(segmentCount == 0));
				}

				// draw segment handlers
				drawSegmentHandler(
					track,
					*segment.get(),
					trackTopLeft,
					segmentX,
					segmentWidth,
					drawList);

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
	}

	template<typename T>
	void SequenceEditorGUIView::drawControlPoints(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{

		const SequenceTrackSegmentCurve<T>& segment 
			= static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		// draw first control point(s) handlers IF this is the first segment of the track
		if (track.mSegments[0]->mID == segment.mID)
		{
			for (int v = 0; v < segment.mCurves.size(); v++)
			{
				const auto& curvePoint = segment.mCurves[v]->mPoints[0];
				std::ostringstream stringStream;
				stringStream << segment.mID << "_point_" << 0 << "_curve_" << v;

				ImVec2 circlePoint =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
					trackTopLeft.y + mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					0,
					v,
					TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					0,
					v,
					TanPointTypes::OUT,
					drawList);
			}
		}

		// draw control points of curves
		// we ignore the first and last because they are controlled by the start & end value of the segment
		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			for (int i = 1; i < segment.mCurves[v]->mPoints.size() - 1; i++)
			{
				// get the curvepoint and generate a unique ID for the control point
				const auto& curvePoint = segment.mCurves[v]->mPoints[i];
				std::ostringstream stringStream;
				stringStream << segment.mID << "_point_" << i << "_curve_" << v;
				std::string pointID = stringStream.str();

				// determine the point at where to draw the control point
				ImVec2 circlePoint =
				{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
					trackTopLeft.y + mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

				// handle mouse hovering
				bool hovered = false;
				if (mIsWindowFocused)
				{
					if ((mEditorAction.currentAction == NONE ||
						mEditorAction.currentAction == HOVERING_CONTROL_POINT ||
						mEditorAction.currentAction == HOVERING_CURVE)
						&& ImGui::IsMouseHoveringRect(
					{ circlePoint.x - 5, circlePoint.y - 5 },
					{ circlePoint.x + 5, circlePoint.y + 5 }))
					{
						hovered = true;
					}
				}

				if (hovered)
				{
					// if we are hovering this point, store ID
					mEditorAction.currentAction = HOVERING_CONTROL_POINT;
					mEditorAction.currentObjectID = pointID;

					//
					showValue<T>(track, segment, curvePoint.mPos.mTime, v);

					// is the mouse held down, then we are dragging
					if (ImGui::IsMouseDown(0))
					{
						mEditorAction.currentAction = DRAGGING_CONTROL_POINT;
						mEditorAction.currentActionData = std::make_unique<SequenceGUIControlPointData>(
							track.mID, 
							segment.mID, 
							i,
							v);
						mEditorAction.currentObjectID = segment.mID;
					}
					// if we clicked right mouse button, delete control point
					else if (ImGui::IsMouseClicked(1))
					{
						mEditorAction.currentAction = DELETE_CONTROL_POINT;
						mEditorAction.currentActionData = std::make_unique<SequenceGUIDeleteControlPointData>(
							track.mID, 
							segment.mID, 
							i,
							v);
						mEditorAction.currentObjectID = segment.mID;
					}
				}
				else
				{
					// otherwise, if we where hovering but not anymore, stop hovering
					if (mEditorAction.currentAction == HOVERING_CONTROL_POINT &&
						pointID == mEditorAction.currentObjectID)
					{
						mEditorAction.currentAction = NONE;
					}
				}

				if (mIsWindowFocused)
				{
					// handle dragging of control point
					if (mEditorAction.currentAction == DRAGGING_CONTROL_POINT &&
						segment.mID == mEditorAction.currentObjectID)
					{
						const SequenceGUIControlPointData* data
							= dynamic_cast<SequenceGUIControlPointData*>(mEditorAction.currentActionData.get());

						if (data->controlPointIndex == i && data->curveIndex == v)
						{
							float timeAdjust = mMouseDelta.x / segmentWidth;
							float valueAdjust = (mMouseDelta.y / mTrackHeight) * -1.0f;

							hovered = true;

							showValue<T>(track, segment, curvePoint.mPos.mTime, v);

							mController.changeCurvePoint<T>(
								data->trackID,
								data->segmentID,
								data->controlPointIndex,
								data->curveIndex,
								timeAdjust,
								valueAdjust);
							mCurveCache.clear();

							if (ImGui::IsMouseReleased(0))
							{
								mEditorAction.currentAction = NONE;
								mEditorAction.currentActionData = nullptr;
							}
						}
					}

					// handle deletion of control point
					if (mEditorAction.currentAction == DELETE_CONTROL_POINT &&
						segment.mID == mEditorAction.currentObjectID)
					{
						const SequenceGUIDeleteControlPointData* data
							= dynamic_cast<SequenceGUIDeleteControlPointData*>(mEditorAction.currentActionData.get());

						if (data->controlPointIndex == i && 
							data->curveIndex == v)
						{
							mController.deleteCurvePoint<T>(
								data->trackID,
								data->segmentID,
								data->controlPointIndex,
								data->curveIndex);
							mCurveCache.clear();

							mEditorAction.currentAction = NONE;
							mEditorAction.currentActionData = nullptr;
						}
					}
				}


				// draw the control point
				drawList->AddCircleFilled(
					circlePoint,
					4.0f,
					hovered ? guicolors::white : guicolors::lightGrey);

				// draw the handlers
				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					i,
					v,
					TanPointTypes::IN,
					drawList);

				drawTanHandler<T>(
					track,
					segment,
					stringStream,
					segmentWidth,
					curvePoint,
					circlePoint,
					i,
					v,
					TanPointTypes::OUT,
					drawList);
			}
		}

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// handle last control point
			// overlaps with endvalue so only draw tan handlers
			const int controlPointIndex = segment.mCurves[v]->mPoints.size() - 1;
			const auto& curvePoint = segment.mCurves[v]->mPoints[controlPointIndex];

			std::ostringstream stringStream;
			stringStream << segment.mID << "_point_" << controlPointIndex << "_curve_" << v;
			std::string pointID = stringStream.str();

			ImVec2 circlePoint =
			{ (trackTopLeft.x + segmentX - segmentWidth) + segmentWidth * curvePoint.mPos.mTime,
				trackTopLeft.y + mTrackHeight * (1.0f - (float)curvePoint.mPos.mValue) };

			drawTanHandler<T>(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				circlePoint,
				controlPointIndex,
				v,
				TanPointTypes::IN,
				drawList);

			drawTanHandler<T>(
				track,
				segment,
				stringStream,
				segmentWidth,
				curvePoint,
				circlePoint,
				controlPointIndex,
				v,
				TanPointTypes::OUT,
				drawList);
		}

		//
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - mTrackHeight);
	}

	template<typename T>
	void SequenceEditorGUIView::drawSegmentValue(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		const SegmentValueTypes segmentType,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment = 
			static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

		for (int v = 0; v < segment.mCurves.size(); v++)
		{
			// calculate point of this value in the window
			ImVec2 segmentValuePos =
			{
				trackTopLeft.x + segmentX - (segmentType == BEGIN ? segmentWidth : 0.0f),
				trackTopLeft.y + mTrackHeight * (1.0f - ((segmentType == BEGIN ? 
				(float)segment.mCurves[v]->mPoints[0].mPos.mValue : 
				(float)segment.mCurves[v]->mPoints[segment.mCurves[v]->mPoints.size()-1].mPos.mValue) / 1.0f))
			};

			bool hovered = false;

			if (mIsWindowFocused)
			{
				// check if we are hovering this value
				if ((mEditorAction.currentAction == SequenceGUIMouseActions::NONE ||
					mEditorAction.currentAction == HOVERING_SEGMENT_VALUE ||
					mEditorAction.currentAction == HOVERING_SEGMENT ||
					mEditorAction.currentAction == HOVERING_CURVE)
					&& ImGui::IsMouseHoveringRect(
						{ segmentValuePos.x - 12, segmentValuePos.y - 12 }, // top left
						{ segmentValuePos.x + 12, segmentValuePos.y + 12 }))  // bottom right 
				{
					hovered = true;
					mEditorAction.currentAction = HOVERING_SEGMENT_VALUE;
					mEditorAction.currentActionData = std::make_unique<SequenceGUIDragSegmentData>(
						track.mID,
						segment.mID,
						segmentType,
						v);

					if (ImGui::IsMouseDown(0))
					{
						mEditorAction.currentAction = DRAGGING_SEGMENT_VALUE;
						mEditorAction.currentObjectID = segment.mID;
					}

					showValue<T>(track, segment, segmentType == BEGIN ? 0.0f : 1.0f, v);
				}
				else if (mEditorAction.currentAction != DRAGGING_SEGMENT_VALUE)
				{
					if (mEditorAction.currentAction == HOVERING_SEGMENT_VALUE)
					{
						const SequenceGUIDragSegmentData* data =
							dynamic_cast<SequenceGUIDragSegmentData*>(mEditorAction.currentActionData.get());

						if (data->type == segmentType && 
							data->segmentID == segment.mID &&
							data->curveIndex == v)
						{
							mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
						}
					}
				}

				// handle dragging segment value
				if (mEditorAction.currentAction == DRAGGING_SEGMENT_VALUE &&
					mEditorAction.currentObjectID == segment.mID)
				{
					const SequenceGUIDragSegmentData* data =
						dynamic_cast<SequenceGUIDragSegmentData*>(mEditorAction.currentActionData.get());

					if (data->type == segmentType && data->curveIndex == v)
					{
						hovered = true;
						showValue<T>(track, segment, segmentType == BEGIN ? 0.0f : 1.0f, v);

						if (ImGui::IsMouseReleased(0))
						{
							mEditorAction.currentAction = NONE;
						}
						else
						{
							float dragAmount = (mMouseDelta.y / mTrackHeight) * -1.0f;
							
							mController.changeCurveSegmentValue<T>(
								track.mID,
								segment.mID,
								dragAmount,
								v,
								segmentType);
							mCurveCache.clear();
						}
					}
				}
			}

			if (hovered)
				drawList->AddCircleFilled(segmentValuePos, 5.0f, guicolors::curvecolors[v]);
			else
				drawList->AddCircle(segmentValuePos, 5.0f, guicolors::curvecolors[v]);
		}
	}


	void SequenceEditorGUIView::drawSegmentHandler(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{
		// segment handler
		if (mIsWindowFocused &&
			(mEditorAction.currentAction == SequenceGUIMouseActions::NONE ||
			(mEditorAction.currentAction == HOVERING_SEGMENT && mEditorAction.currentObjectID == segment.mID)) &&
			ImGui::IsMouseHoveringRect(
		{ trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 }, // top left
		{ trackTopLeft.x + segmentX + 10, trackTopLeft.y + mTrackHeight + 10 }))  // bottom right 
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

						// we are hovering this segment with the mouse
			mEditorAction.currentAction = HOVERING_SEGMENT;
			mEditorAction.currentObjectID = segment.mID;

			// left mouse is start dragging
			if (ImGui::IsMouseDown(0))
			{
				mEditorAction.currentAction = SequenceGUIMouseActions::DRAGGING_SEGMENT;
				mEditorAction.currentObjectID = segment.mID;
			}
			// right mouse in deletion popup
			else if (ImGui::IsMouseDown(1))
			{
				std::unique_ptr<SequenceGUIDeleteSegmentData> deleteSegmentData = std::make_unique<SequenceGUIDeleteSegmentData>(track.mID, segment.mID);
				mEditorAction.currentAction = SequenceGUIMouseActions::OPEN_DELETE_SEGMENT_POPUP;
				mEditorAction.currentObjectID = segment.mID;
				mEditorAction.currentActionData = std::move(deleteSegmentData);
			}
		}
		else if (
			mEditorAction.currentAction == SequenceGUIMouseActions::DRAGGING_SEGMENT &&
			mEditorAction.currentObjectID == segment.mID)
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			// do we have the mouse still held down ? drag the segment
			if (ImGui::IsMouseDown(0))
			{
				float amount = mMouseDelta.x / mStepSize;
				if (track.getTrackType() == SequenceTrackTypes::EVENT)
				{
					mController.segmentEventStartTimeChange(track.mID, segment.mID, amount);
				}
				else
				{
					mController.segmentDurationChange(segment.mID, amount);
				}
				mCurveCache.clear();
			}
			// otherwise... release!
			else if (ImGui::IsMouseReleased(0))
			{
				mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
			}
		}
		else
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mTrackHeight }, // bottom right
				guicolors::white, // color
				1.0f); // thickness

			// release if we are not hovering this segment
			if (mEditorAction.currentAction == HOVERING_SEGMENT
				&& mEditorAction.currentObjectID == segment.mID)
			{
				mEditorAction.currentAction = NONE;
			}
		}
		
	}

	template<typename T>
	void SequenceEditorGUIView::drawTanHandler(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		std::ostringstream &stringStream,
		const float segmentWidth,
		const math::FCurvePoint<float, float> &curvePoint,
		const ImVec2 &circlePoint,
		const int controlPointIndex,
		const int curveIndex,
		const TanPointTypes type,
		ImDrawList* drawList)
	{
		// draw tan handlers
		{
			// create a string stream to create identifier of this object
			std::ostringstream tanStream;
			tanStream << stringStream.str() << (type == TanPointTypes::IN) ? "inTan" : "outTan";

			//
			const math::FComplex<float, float>& tanComplex
				= (type == TanPointTypes::IN) ? curvePoint.mInTan : curvePoint.mOutTan;

			// get the offset from the tan
			ImVec2 offset =
			{ (segmentWidth * tanComplex.mTime) / (float)segment.mDuration,
				(mTrackHeight *  (float)tanComplex.mValue * -1.0f) };
			ImVec2 tanPoint = { circlePoint.x + offset.x, circlePoint.y + offset.y };

			// set if we are hoverting this point with the mouse
			bool tanPointHovered = false;

			if (mIsWindowFocused)
			{
				// check if hovered
				if ((mEditorAction.currentAction == NONE ||
					mEditorAction.currentAction == HOVERING_CURVE)
					&& ImGui::IsMouseHoveringRect(
				{ tanPoint.x - 5, tanPoint.y - 5 },
				{ tanPoint.x + 5, tanPoint.y + 5 }))
				{
					mEditorAction.currentAction = HOVERING_TAN_POINT;
					mEditorAction.currentObjectID = tanStream.str();
					tanPointHovered = true;
				}
				else if (
					mEditorAction.currentAction == HOVERING_TAN_POINT)
				{
					// if we hare already hovering, check if its this point
					if (mEditorAction.currentObjectID == tanStream.str())
					{
						if (ImGui::IsMouseHoveringRect(
						{ tanPoint.x - 5, tanPoint.y - 5 },
						{ tanPoint.x + 5, tanPoint.y + 5 }))
						{
							// still hovering
							tanPointHovered = true;

							// start dragging if mouse down
							if (ImGui::IsMouseDown(0) && mEditorAction.currentAction == HOVERING_TAN_POINT)
							{
								mEditorAction.currentAction = DRAGGING_TAN_POINT;

								mEditorAction.currentActionData = std::make_unique<SequenceGUIDragTanPointData>(
									track.mID,
									segment.mID,
									controlPointIndex,
									curveIndex,
									type);
							}
						}
						else
						{
							// otherwise, release!
							mEditorAction.currentAction = NONE;
						}
					}
				}

				// handle dragging of tan point
				if (mEditorAction.currentAction == DRAGGING_TAN_POINT)
				{
					const SequenceGUIDragTanPointData* data =
						dynamic_cast<SequenceGUIDragTanPointData*>(mEditorAction.currentActionData.get());

					if (data->segmentID == segment.mID && 
						data->controlPointIndex == controlPointIndex &&
						data->type == type &&
						data->curveIndex == curveIndex)
					{
						if (ImGui::IsMouseReleased(0))
						{
							mEditorAction.currentAction = NONE;
							mEditorAction.currentActionData = nullptr;
						}
						else
						{
							tanPointHovered = true;

							float time = mMouseDelta.x / mStepSize;
							float value = (mMouseDelta.y / mTrackHeight) * -1.0f;

							mController.changeTanPoint<T>(
								track.mID,
								segment.mID,
								controlPointIndex,
								curveIndex,
								type,
								time,
								value);
							mCurveCache.clear();
						}
					}
				}
			}

			// draw line
			drawList->AddLine(circlePoint, tanPoint, tanPointHovered ? guicolors::white : guicolors::darkGrey, 1.0f);

			// draw handler
			drawList->AddCircleFilled(tanPoint, 3.0f, tanPointHovered ? guicolors::white : guicolors::darkGrey);
		}
	}


	void SequenceEditorGUIView::handleInsertSegmentPopup()
	{
		if (mEditorAction.currentAction == OPEN_INSERT_SEGMENT_POPUP)
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Segment");

			mEditorAction.currentAction = INSERTING_SEGMENT;
		}

		// handle insert segment popup
		if (mEditorAction.currentAction == INSERTING_SEGMENT)
		{
			if (ImGui::BeginPopup("Insert Segment"))
			{
				if (ImGui::Button("Insert"))
				{
					const SequenceGUIInsertSegmentData* data = dynamic_cast<SequenceGUIInsertSegmentData*>(mEditorAction.currentActionData.get());
					switch (data->trackType)
					{
					case SequenceTrackTypes::FLOAT:
					{
						mController.insertCurveSegment<float>(data->trackID, data->time);
						mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
						mEditorAction.currentActionData = nullptr;
					}
						break;
					case SequenceTrackTypes::VEC4:
					{
						mController.insertCurveSegment<glm::vec4>(data->trackID, data->time);
						mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
						mEditorAction.currentActionData = nullptr;
					}
						break;;
					case SequenceTrackTypes::VEC3:
					{
						mController.insertCurveSegment<glm::vec3>(data->trackID, data->time);
						mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
						mEditorAction.currentActionData = nullptr;
					}
						break;
					case SequenceTrackTypes::VEC2:
					{
						mController.insertCurveSegment<glm::vec2>(data->trackID, data->time);
						mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
						mEditorAction.currentActionData = nullptr;
					}
						break;
					case SequenceTrackTypes::EVENT:
					{
						mEditorAction.currentAction = OPEN_INSERT_EVENT_SEGMENT_POPUP;
						mEditorAction.currentActionData = std::make_unique<SequenceGUIInsertEventSegment>(data->trackID, data->time);
					}
						
						break;
					}
					
					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
	
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
					mEditorAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
				mEditorAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceEditorGUIView::handleInsertTrackPopup()
	{
		if (mEditorAction.currentAction == OPEN_INSERT_TRACK_POPUP)
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Track");

			mEditorAction.currentAction = INSERTING_TRACK;
		}

		// handle insert segment popup
		if (mEditorAction.currentAction == INSERTING_TRACK)
		{
			if (ImGui::BeginPopup("Insert Track"))
			{
				bool closePopup = false;

				if (ImGui::Button("Vector 4"))
				{
					mController.addNewCurveTrack<glm::vec4>();
					mCurveCache.clear();
					closePopup = true;
				}

				if (ImGui::Button("Vector 3"))
				{
					mController.addNewCurveTrack<glm::vec3>();
					mCurveCache.clear();
					closePopup = true;
				}

				if (ImGui::Button("Vector 2"))
				{
					mController.addNewCurveTrack<glm::vec2>();
					mCurveCache.clear();
					closePopup = true;
				}

				if (ImGui::Button("Float"))
				{
					mController.addNewCurveTrack<float>();
					mCurveCache.clear();
					closePopup = true;
				}

				if (ImGui::Button("Event"))
				{
					mController.addNewEventTrack();
					mCurveCache.clear();
					closePopup = true;
				}

				if (ImGui::Button("Cancel"))
				{
					closePopup = true;
				}

				if (closePopup)
				{
					ImGui::CloseCurrentPopup();
					mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
					mEditorAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
				mEditorAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceEditorGUIView::handleInsertEventSegmentPopup()
	{
		if (mEditorAction.currentAction == OPEN_INSERT_EVENT_SEGMENT_POPUP)
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Event");

			mEditorAction.currentAction = INSERTING_EVENT_SEGMENT;
		}

		// handle insert segment popup
		if (mEditorAction.currentAction == INSERTING_EVENT_SEGMENT)
		{
			if (ImGui::BeginPopup("Insert Event"))
			{
				SequenceGUIInsertEventSegment& data = static_cast<SequenceGUIInsertEventSegment&>(*mEditorAction.currentActionData.get());
				
				int n = data.eventMessage.length();
				char buffer[256];
				strcpy(buffer, data.eventMessage.c_str());

				if (ImGui::InputText("message", buffer, 256))
				{
					data.eventMessage = std::string(buffer);
				}

				if (ImGui::Button("Insert"))
				{
					mController.insertEventSegment(data.trackID, data.time, data.eventMessage);
					mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
					mEditorAction.currentActionData = nullptr;
				}


				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
					mEditorAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
				mEditorAction.currentActionData = nullptr;
			}
		}
	}

	void SequenceEditorGUIView::handleDeleteSegmentPopup()
	{
		if (mEditorAction.currentAction == OPEN_DELETE_SEGMENT_POPUP)
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Delete Segment");

			mEditorAction.currentAction = DELETING_SEGMENT;
		}

		// handle delete segment popup
		if (mEditorAction.currentAction == DELETING_SEGMENT)
		{
			if (ImGui::BeginPopup("Delete Segment"))
			{
				if (ImGui::Button("Delete"))
				{
					const SequenceGUIDeleteSegmentData* data = dynamic_cast<SequenceGUIDeleteSegmentData*>(mEditorAction.currentActionData.get());
					mController.deleteSegment(data->trackID, data->segmentID);
					mCurveCache.clear();

					ImGui::CloseCurrentPopup();
					mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
					mEditorAction.currentActionData = nullptr;
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
					mEditorAction.currentActionData = nullptr;
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mEditorAction.currentAction = SequenceGUIMouseActions::NONE;
				mEditorAction.currentActionData = nullptr;
			}
		}
	}


	void SequenceEditorGUIView::drawPlayerController(SequencePlayer& player)
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
						double timeInPlayer = mController.getSequence().mDuration * (float)((float)i / steps);
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
				if (mEditorAction.currentAction == NONE || mEditorAction.currentAction == HOVERING_PLAYER_TIME)
				{
					if (ImGui::IsMouseHoveringRect(startPos, 
					{
						startPos.x + mTimelineWidth,
						startPos.y + timelineControllerHeight
					}))
					{
						mEditorAction.currentAction = HOVERING_PLAYER_TIME;

						if (ImGui::IsMouseDown(0))
						{
							//
							bool playerWasPlaying = player.getIsPlaying();
							bool playerWasPaused = player.getIsPaused();

							mEditorAction.currentAction = DRAGGING_PLAYER_TIME;
							mEditorAction.currentActionData = std::make_unique<SequenceGUIDragPlayerData>(
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
						
						mEditorAction.currentAction = NONE;
					}
				}else if (mEditorAction.currentAction == DRAGGING_PLAYER_TIME)
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
							const SequenceGUIDragPlayerData* data = dynamic_cast<SequenceGUIDragPlayerData*>( mEditorAction.currentActionData.get() );
							if (data->playerWasPlaying && !data->playerWasPaused)
							{
								player.setIsPlaying(true);
							}

							mEditorAction.currentAction = NONE;
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
			mTimelineControllerPos.x
				+ mInspectorWidth + 5 
				+ mTimelineWidth * (float)(player.getPlayerTime() / player.getDuration()) - 1,
			mTimelineControllerPos.y + 15.0f
		});

		ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, guicolors::red);
		if (ImGui::BeginChild(
			idString.c_str(), // id
			{ 1.0f, sequence.mTracks.size() * ( mVerticalResolution + 10.0f ) + 10.0f }, // size
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
		if (mEditorAction.currentAction == LOAD)
		{
			//
			if (ImGui::BeginPopupModal(
				"Load",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				SequenceGUILoadShowData* data = dynamic_cast<SequenceGUILoadShowData*>(mEditorAction.currentActionData.get());

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
					if (mController.getSequencePlayer().load(
						showFiles[data->selectedShow], errorState))
					{
						mEditorAction.currentAction = NONE;
						mEditorAction.currentActionData = nullptr;
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
					mEditorAction.currentAction = NONE;
					mEditorAction.currentActionData = nullptr;
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text(data->errorString.c_str());
					if (ImGui::Button("OK"))
					{
						mEditorAction.currentAction = LOAD;
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
		if (mEditorAction.currentAction == SAVE_AS)
		{
			// save as popup
			if (ImGui::BeginPopupModal(
				"Save As",
				nullptr,
				ImGuiWindowFlags_AlwaysAutoResize))
			{
				//
				SequenceGUISaveShowData* data =
					dynamic_cast<SequenceGUISaveShowData*>(mEditorAction.currentActionData.get());

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

				if (Combo("Shows",
					&data->selectedShow,
					shows))
				{
					if (data->selectedShow == shows.size() - 1)
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
					if (mController.getSequencePlayer().save(showDir + "/" + newShowFileName, errorState))
					{
						data->selectedShow = shows.size() - 2;
					}
					else
					{
						data->errorString = errorState.toString();
						ImGui::OpenPopup("Error");
					}
				}

				if (ImGui::BeginPopupModal("Overwrite"))
				{
					utility::ErrorState errorState;
					ImGui::Text(("Are you sure you want to overwrite " + 
						shows[data->selectedShow] + " ?").c_str());
					if (ImGui::Button("OK"))
					{
						if (mController.getSequencePlayer().save(
							shows[data->selectedShow],
							errorState))
						{
						}
						else
						{
							data->errorString = errorState.toString();
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
					ImGui::Text(data->errorString.c_str());
					if (ImGui::Button("OK"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Done"))
				{
					mEditorAction.currentAction = NONE;
					mEditorAction.currentActionData = nullptr;
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
	}


	template<typename T>
	void SequenceEditorGUIView::drawCurves(
		const SequenceTrack& track,
		const SequenceTrackSegment& segmentBase,
		const ImVec2 &trackTopLeft,
		const float previousSegmentX,
		const float segmentWidth,
		const float segmentX,
		ImDrawList* drawList)
	{
		const SequenceTrackSegmentCurve<T>& segment 
			= static_cast<const SequenceTrackSegmentCurve<T>&>(segmentBase);

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

					points[i + v * (resolution+1)] =
					{
						trackTopLeft.x + previousSegmentX + segmentWidth * ((float)i / resolution),
						trackTopLeft.y + value * mTrackHeight
					};
				}
			}
			mCurveCache.emplace(segment.mID, points);
		}

		int selectedCurve = -1;
		if (mIsWindowFocused)
		{
			// determine if mouse is hovering curve
			if ((mEditorAction.currentAction == NONE || 
				 mEditorAction.currentAction == HOVERING_CURVE)
					&& ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - segmentWidth, trackTopLeft.y }, // top left
					{ trackTopLeft.x + segmentX, trackTopLeft.y + mTrackHeight }))  // bottom right 
			{
				// translate mouse position to position in curve
				ImVec2 mousePos = ImGui::GetMousePos();
				float xInSegment = ((mousePos.x - (trackTopLeft.x + segmentX - segmentWidth)) / mStepSize) / segment.mDuration;
				float yInSegment = 1.0f - ((mousePos.y - trackTopLeft.y) / mTrackHeight);

				for (int i = 0; i < segment.mCurves.size(); i++)
				{
					// evaluate curve at x position
					float yInCurve = segment.mCurves[i]->evaluate(xInSegment);

					// insert curve point on click
					const float maxDist = 0.1f;
					if (abs(yInCurve - yInSegment) < maxDist)
					{
						mEditorAction.currentAction = HOVERING_CURVE;
						mEditorAction.currentObjectID = segment.mID;
						mEditorAction.currentActionData = std::make_unique<SequenceGUIHoveringCurveData>(i);
						if (ImGui::IsMouseClicked(0))
						{
							mController.insertCurvePoint<T>(
								track.mID,
								segment.mID,
								xInSegment, 
								i);
						}
						selectedCurve = i;
					}
				}

				if (selectedCurve == -1)
				{
					mEditorAction.currentAction = NONE;
					mEditorAction.currentActionData = nullptr;
				}
				else
				{
					showValue<T>(track, segment, xInSegment, selectedCurve);
				}
			}
			else
			{
				if (mEditorAction.currentAction == HOVERING_CURVE &&
					mEditorAction.currentObjectID == segment.mID )
				{
					mEditorAction.currentAction = NONE;
					mEditorAction.currentActionData = nullptr;
				}
			}
		}

		for (int i = 0; i < segment.mCurves.size(); i++)
		{
			// draw points of curve
			drawList->AddPolyline(
				&*mCurveCache[segment.mID].begin() + i * ( resolution + 1 ), // points array
				mCurveCache[segment.mID].size() / segment.mCurves.size(), // size of points array
				guicolors::curvecolors[i], // color
				false, // closed
				selectedCurve == i ? 3.0f : 1.0f, // thickness
				true); // anti-aliased
		}
	}

	template<typename T>
	void SequenceEditorGUIView::drawSegmentContent(
		const SequenceTrack &track,
		const SequenceTrackSegment &segment,
		const ImVec2& trackTopLeft,
		float previousSegmentX,
		float segmentWidth,
		float segmentX,
		ImDrawList* drawList,
		bool drawStartValue)
	{
		// curve
		drawCurves<T>(
			track,
			segment,
			trackTopLeft,
			previousSegmentX,
			segmentWidth,
			segmentX,
			drawList);
			

		// draw control points
		drawControlPoints<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
			drawList);

		// if this is the first segment of the track
		// also draw a handler for the start value
		if (drawStartValue)
		{
			// draw segment value handler
			drawSegmentValue<T>(
				track,
				segment,
				trackTopLeft,
				segmentX,
				segmentWidth,
				SegmentValueTypes::BEGIN,
				drawList);
		}

		// draw segment value handler
		drawSegmentValue<T>(
			track,
			segment,
			trackTopLeft,
			segmentX,
			segmentWidth,
			SegmentValueTypes::END,
			drawList);
	}


	template<typename T>
	void SequenceEditorGUIView::drawInspectorRange(const SequenceTrack& track)
	{
		const SequenceTrackCurve<T>& curveTrack = static_cast<const SequenceTrackCurve<T>&>(track);

		T min = curveTrack.mMinimum;
		T max = curveTrack.mMaximum;

		//
		ImGui::PushID(track.mID.c_str());

		float dragFloatX = ImGui::GetCursorPosX() + 40;
		ImGui::SetCursorPos({ ImGui::GetCursorPosX() + 5, ImGui::GetCursorPosY() + 5 });
		ImGui::Text("Min:"); ImGui::SameLine();
		ImGui::PushID("min");
		ImGui::SetCursorPosX(dragFloatX);
		if (inputFloat<T>(min, 3))
		{
			mController.changeMinMaxCurveTrack<T>(track.mID, min, max);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);
		ImGui::Text("Max:"); ImGui::SameLine();
		ImGui::PushID("max");
		ImGui::SetCursorPosX(dragFloatX);
		if (inputFloat<T>(max, 3))
		{
			mController.changeMinMaxCurveTrack<T>(track.mID, min, max);
		}
		ImGui::PopID();
		ImGui::PopItemWidth();

		ImGui::PopID();
	}

	template<typename T>
	void SequenceEditorGUIView::showValue(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<T>& segment,
		float x,
		int curveIndex)
	{
		assert(false);
	}

	template<>
	void SequenceEditorGUIView::showValue<float>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<float>& segment,
		float x,
		int curveIndex)
	{
		const SequenceTrackCurve<float>& curveTrack = static_cast<const SequenceTrackCurve<float>&>(track);

		ImGui::BeginTooltip();

		ImGui::Text("Time : %.3f", mMouseCursorTime);
		ImGui::Text("%.3f", segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceEditorGUIView::showValue<glm::vec2>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec2>& segment,
		float x,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 2);

		const SequenceTrackCurve<glm::vec2>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec2>&>(track);

		ImGui::BeginTooltip();

		glm::vec2 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[2] =
		{
			"x",
			"y"
		};

		ImGui::Text("Time : %.3f", mMouseCursorTime);
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceEditorGUIView::showValue<glm::vec3>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec3>& segment,
		float x,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 3);

		const SequenceTrackCurve<glm::vec3>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec3>&>(track);

		ImGui::BeginTooltip();

		glm::vec3 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[3] =
		{
			"x",
			"y",
			"z"
		};

		ImGui::Text("Time : %.3f", mMouseCursorTime);
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<>
	void SequenceEditorGUIView::showValue<glm::vec4>(
		const SequenceTrack& track,
		const SequenceTrackSegmentCurve<glm::vec4>& segment,
		float x,
		int curveIndex)
	{
		assert(curveIndex >= 0);
		assert(curveIndex < 4);

		const SequenceTrackCurve<glm::vec4>& curveTrack = static_cast<const SequenceTrackCurve<glm::vec4>&>(track);

		ImGui::BeginTooltip();

		glm::vec4 value = segment.getValue(x) * (curveTrack.mMaximum - curveTrack.mMinimum) + curveTrack.mMinimum;

		static std::string names[4] =
		{
			"x",
			"y",
			"z",
			"w"
		};

		ImGui::Text("Time : %.3f", mMouseCursorTime);
		ImGui::Text("%s : %.3f", names[curveIndex].c_str(), value[curveIndex]);

		ImGui::EndTooltip();
	}

	template<typename T>
	bool SequenceEditorGUIView::inputFloat(T &v, int precision)
	{
		assert(true);
		return false;
	}

	template<>
	bool SequenceEditorGUIView::inputFloat<float>(float &v, int precision)
	{
		ImGui::PushItemWidth(100.0f);
		return ImGui::InputFloat("", &v, 0.0f, 0.0f, precision);
	}

	template<>
	bool SequenceEditorGUIView::inputFloat<glm::vec2>(glm::vec2 &v, int precision)
	{
		ImGui::PushItemWidth(145.0f);
		return ImGui::InputFloat2("", &v[0], precision);
	}

	template<>
	bool SequenceEditorGUIView::inputFloat<glm::vec3>(glm::vec3 &v, int precision)
	{
		ImGui::PushItemWidth(180.0f);
		return ImGui::InputFloat3("", &v[0], precision);
	}

	template<>
	bool SequenceEditorGUIView::inputFloat<glm::vec4>(glm::vec4 &v, int precision)
	{
		ImGui::PushItemWidth(225.0f);
		return ImGui::InputFloat4("", &v[0], precision);
	}

	std::string SequenceEditorGUIView::formatTimeString(double time)
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

	static bool vector_getter(void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};

	bool SequenceEditorGUIView::Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	bool SequenceEditorGUIView::ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ImGui::ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}
}
