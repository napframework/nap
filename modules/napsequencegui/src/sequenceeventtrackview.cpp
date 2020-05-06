#include "sequenceeventtrackview.h"
#include "sequenceeditorgui.h"
#include "sequencetrackevent.h"
#include "sequencecontrollerevent.h"
#include "napcolors.h"
#include "sequenceplayereventinput.h"
#include "sequencecontrollerevent.h"

#include <nap/logger.h>
#include <iostream>

namespace nap
{
	using namespace SequenceGUIActions;

	SequenceEventTrackView::SequenceEventTrackView(SequenceEditorGUIView& view)
		: SequenceTrackView(view)
	{
		nap::Logger::info("event track");
	}


	static bool trackViewRegistration = SequenceEditorGUIView::registerTrackViewType(RTTI_OF(SequenceTrackEvent), RTTI_OF(SequenceEventTrackView));

	static bool registerCurveTrackView = SequenceTrackView::registerFactory(RTTI_OF(SequenceEventTrackView), [](SequenceEditorGUIView& view)->std::unique_ptr<SequenceTrackView>
	{
		return std::make_unique<SequenceEventTrackView>(view);
	});

	void SequenceEventTrackView::drawTrack(const SequenceTrack& track, SequenceEditorGUIState& state)
	{
		std::string deleteTrackID;
		bool deleteTrack = false;

		mState = &state;

		drawEventTrack(track, mState->mCursorPos, 10.0f, getPlayer(), deleteTrack, deleteTrackID);

		if (deleteTrack)
		{
			auto& controller = getEditor().getController<SequenceControllerEvent>();
			controller.deleteTrack(track.mID);
		}
	}


	void SequenceEventTrackView::handlePopups(SequenceEditorGUIState& state)
	{
		handleInsertEventSegmentPopup();

		handleDeleteSegmentPopup();

		handleEditEventSegmentPopup();
	}


	void SequenceEventTrackView::drawEventTrack(
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
			mState->mTrackHeight + marginBetweenTracks + cursorPos.y
		};

		// manually set the cursor position before drawing inspector
		ImVec2 inspectorCursorPos = { cursorPos.x , cursorPos.y };
		ImGui::SetCursorPos(inspectorCursorPos);

		// draw inspector window
		if (ImGui::BeginChild(
			inspectorID.c_str(), // id
			{ mState->mInspectorWidth , mState->mTrackHeight + 5 }, // size
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
				{ windowPos.x + windowSize.x - 5, windowPos.y + mState->mTrackHeight },
				guicolors::black);

			drawList->AddRect(
				windowPos,
				{ windowPos.x + windowSize.x - 5, windowPos.y + mState->mTrackHeight },
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
			std::vector<std::string> eventInputs;
			int currentItem = 0;
			eventInputs.emplace_back("none");
			int count = 0;
			const SequenceEventReceiver* assignedEventReceiver = nullptr;
			
			for (const auto& input : sequencePlayer.mInputs)
			{
				if (input.get()->get_type() == RTTI_OF(SequencePlayerEventInput))
				{
					count++;

					if (input->mID == track.mAssignedObjectIDs)
					{
						assigned = true;
						assignedID = input->mID;
						currentItem = count;

						assert(input.get()->get_type() == RTTI_OF(SequencePlayerEventInput)); // type mismatch
						assignedEventReceiver = static_cast<SequencePlayerEventInput*>(input.get())->mReceiver.get();
					}

					eventInputs.emplace_back(input->mID);
				}
			}

			ImGui::PushItemWidth(140.0f);
			if (Combo(
				"",
				&currentItem,
				eventInputs))
			{
				auto& eventController = getEditor().getController<SequenceControllerEvent>();
				
				if (currentItem != 0)
					eventController.assignNewObjectID(track.mID, eventInputs[currentItem]);
				else
					eventController.assignNewObjectID(track.mID, "");

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

		const ImVec2 windowCursorPos = { cursorPos.x + mState->mInspectorWidth + 5, cursorPos.y };
		ImGui::SetCursorPos(windowCursorPos);

		// begin track
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mState->mTimelineWidth + 5 , mState->mTrackHeight + 5 }, // size
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
				{ trackTopLeft.x + mState->mTimelineWidth, trackTopLeft.y + mState->mTrackHeight }, // bottom right position
				guicolors::black); // color 

								   // draw border of track
			drawList->AddRect(
				trackTopLeft, // top left position
				{ trackTopLeft.x + mState->mTimelineWidth, trackTopLeft.y + mState->mTrackHeight }, // bottom right position
				guicolors::white); // color 

								   //
			mState->mMouseCursorTime = (mState->mMousePos.x - trackTopLeft.x) / mState->mStepSize;

			if (mState->mIsWindowFocused)
			{
				// handle insertion of segment
				if (mState->mAction->isAction<None>())
				{
					if (ImGui::IsMouseHoveringRect(
						trackTopLeft, // top left position
						{ trackTopLeft.x + mState->mTimelineWidth, trackTopLeft.y + mState->mTrackHeight }))
					{
						// position of mouse in track
						drawList->AddLine(
						{ mState->mMousePos.x, trackTopLeft.y }, // top left
						{ mState->mMousePos.x, trackTopLeft.y + mState->mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness

						ImGui::BeginTooltip();
						ImGui::Text(formatTimeString(mState->mMouseCursorTime).c_str());
						ImGui::EndTooltip();

						// right mouse down
						if (ImGui::IsMouseClicked(1))
						{
							double time = mState->mMouseCursorTime;

							//
							mState->mAction = createAction<OpenInsertEventSegmentPopup>(
								track.mID, 
								time);
						}
					}
				}

				// draw line in track while in inserting segment popup
				if (mState->mAction->isAction<OpenInsertEventSegmentPopup>())
				{
					auto* action = mState->mAction->getDerived<OpenInsertEventSegmentPopup>();
					if (action->mTrackID == track.mID)
					{
						// position of insertion in track
						drawList->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState->mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState->mStepSize, trackTopLeft.y + mState->mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness
					}
				}

				if ( mState->mAction->isAction<InsertingEventSegment>())
				{
					auto* action = mState->mAction->getDerived<InsertingEventSegment>();
					if (action->mTrackID == track.mID)
					{
						// position of insertion in track
						drawList->AddLine(
						{ trackTopLeft.x + (float)action->mTime * mState->mStepSize, trackTopLeft.y }, // top left
						{ trackTopLeft.x + (float)action->mTime * mState->mStepSize, trackTopLeft.y + mState->mTrackHeight }, // bottom right
							guicolors::lightGrey, // color
							1.0f); // thickness
					}
				}
			}

			float previousSegmentX = 0.0f;

			int segmentCount = 0;
			for (const auto& segment : track.mSegments)
			{
				float segmentX = (segment->mStartTime) * mState->mStepSize;
				float segmentWidth = segment->mDuration * mState->mStepSize;

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


	void SequenceEventTrackView::handleInsertEventSegmentPopup()
	{
		if (mState->mAction->isAction<OpenInsertEventSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Insert Event");

			auto* action = mState->mAction->getDerived<OpenInsertEventSegmentPopup>();
			mState->mAction = createAction<InsertingEventSegment>(action->mTrackID, action->mTime);
		}

		// handle insert segment popup
		if (mState->mAction->isAction<InsertingEventSegment>())
		{
			if (ImGui::BeginPopup("Insert Event"))
			{
				auto* action = mState->mAction->getDerived<InsertingEventSegment>();

				int n = action->mMessage.length();
				char buffer[256];
				strcpy(buffer, action->mMessage.c_str());

				if (ImGui::InputText("message", buffer, 256))
				{
					action->mMessage = std::string(buffer);
				}

				if (ImGui::Button("Insert"))
				{
					auto& eventController = getEditor().getController<SequenceControllerEvent>();
					eventController.insertSegment(action->mTrackID, action->mTime);
					mState->mAction = createAction<None>();
				}


				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction = createAction<None>();
			}
		}
	}


	void SequenceEventTrackView::drawSegmentHandler(
		const SequenceTrack& track,
		const SequenceTrackSegment& segment,
		const ImVec2 &trackTopLeft,
		const float segmentX,
		const float segmentWidth,
		ImDrawList* drawList)
	{
		// segment handler
		if (mState->mIsWindowFocused && ImGui::IsMouseHoveringRect(
					{ trackTopLeft.x + segmentX - 10, trackTopLeft.y - 10 }, // top left
					{ trackTopLeft.x + segmentX + 10, trackTopLeft.y + mState->mTrackHeight + 10 }))  // bottom right 
		{
			bool isAlreadyHovering = false;
			if (mState->mAction->isAction<HoveringSegment>())
			{
				isAlreadyHovering = mState->mAction->getDerived<HoveringSegment>()->mSegmentID == segment.mID;
			}

			bool isAlreadyDragging = false;
			if (mState->mAction->isAction<DraggingSegment>())
			{
				isAlreadyDragging = mState->mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID;
			}

			// draw handler of segment duration
			drawList->AddLine(
				{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
				{ trackTopLeft.x + segmentX, trackTopLeft.y + mState->mTrackHeight }, // bottom right
				guicolors::white, // color
				3.0f); // thickness

			if (!isAlreadyHovering && !isAlreadyDragging)
			{
				// we are hovering this segment with the mouse
				mState->mAction = createAction<HoveringSegment>(track.mID, segment.mID);
			}

			ImGui::BeginTooltip();
			ImGui::Text(formatTimeString(segment.mStartTime).c_str());
			ImGui::EndTooltip();

			// left mouse is start dragging
			if (!isAlreadyDragging)
			{
				if (ImGui::IsMouseDown(0))
				{
					mState->mAction = createAction<DraggingSegment>(track.mID, segment.mID);
				}
			}
			else
			{
				if (ImGui::IsMouseDown(0))
				{
					float amount = mState->mMouseDelta.x / mState->mStepSize;

					auto& editor = getEditor();
					SequenceControllerEvent& eventController = editor.getController<SequenceControllerEvent>();
					eventController.segmentEventStartTimeChange(track.mID, segment.mID, amount);
				}
			}

			// right mouse in deletion popup
			if (ImGui::IsMouseDown(1))
			{
				mState->mAction = createAction<OpenEditSegmentPopup>(track.mID, segment.mID, segment.get_type());
			}
		}
		else 
		{
			// draw handler of segment duration
			drawList->AddLine(
			{ trackTopLeft.x + segmentX, trackTopLeft.y }, // top left
			{ trackTopLeft.x + segmentX, trackTopLeft.y + mState->mTrackHeight }, // bottom right
				guicolors::white, // color
				1.0f); // thickness

			if (mState->mAction->isAction<HoveringSegment>())
			{
				auto* action = mState->mAction->getDerived<HoveringSegment>();
				if (action->mSegmentID == segment.mID)
				{
					mState->mAction = createAction<None>();
				}
			}
		}
		
		if (ImGui::IsMouseReleased(0))
		{
			if (mState->mAction->isAction<DraggingSegment>())
			{
				if (mState->mAction->getDerived<DraggingSegment>()->mSegmentID == segment.mID)
				{
					mState->mAction = createAction<None>();
				}
			}
		}
	}

	
	void SequenceEventTrackView::handleEditEventSegmentPopup()
	{
		if (mState->mAction->isAction<OpenEditEventSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Edit Event");

			auto* action = mState->mAction->getDerived<OpenEditEventSegmentPopup>();
			mState->mAction = createAction<EditingEventSegment>(action->mTrackID, action->mSegmentID, action->mWindowPos);
		}

		// handle insert segment popup
		if (mState->mAction->isAction<EditingEventSegment>())
		{
			if (ImGui::BeginPopup("Edit Event"))
			{
				auto* action = mState->mAction->getDerived<EditingEventSegment>();

				ImGui::SetWindowPos(action->mWindowPos);

				int n = action->mMessage.length();
				char buffer[256];
				strcpy(buffer, action->mMessage.c_str());

				if (ImGui::InputText("message", buffer, 256))
				{
					action->mMessage = std::string(buffer);
				}

				if (ImGui::Button("Done"))
				{
					auto& eventController = getEditor().getController<SequenceControllerEvent>();
					eventController.editEventSegment(action->mTrackID, action->mSegmentID, action->mMessage);
					mState->mAction = createAction<None>();
				}


				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction = createAction<None>();
			}
		}
	}

	
	void SequenceEventTrackView::handleDeleteSegmentPopup()
	{
		if (mState->mAction->isAction<OpenEditSegmentPopup>())
		{
			// invoke insert sequence popup
			ImGui::OpenPopup("Delete Segment");

			auto* action = mState->mAction->getDerived<OpenEditSegmentPopup>();
			mState->mAction = createAction<EditingSegment>(action->mTrackID, action->mSegmentID, action->mSegmentType);
		}

		// handle delete segment popup
		if (mState->mAction->isAction<EditingSegment>())
		{
			if (ImGui::BeginPopup("Delete Segment"))
			{
				auto* action = mState->mAction->getDerived<EditingSegment>();
				if (ImGui::Button("Delete"))
				{
					auto& controller = getEditor().getController<SequenceControllerEvent>();
					controller.deleteSegment(action->mTrackID, action->mSegmentID);
					mState->mDirty = true;

					ImGui::CloseCurrentPopup();
					mState->mAction = createAction<None>();
				}
				else
				{
					if (action->mSegmentType == RTTI_OF(SequenceTrackSegmentEvent))
					{
						if (ImGui::Button("Edit"))
						{
							auto& eventController = getEditor().getController<SequenceControllerEvent>();
							const SequenceTrackSegmentEvent *eventSegment = dynamic_cast<const SequenceTrackSegmentEvent*>(eventController.getSegment(action->mTrackID, action->mSegmentID));
							assert(eventSegment != nullptr);

							mState->mAction = createAction<OpenEditEventSegmentPopup>(
								action->mTrackID,
								action->mSegmentID,
								ImGui::GetWindowPos(),
								eventSegment->mMessage);

							ImGui::CloseCurrentPopup();
						}
					}
				}

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
					mState->mAction = createAction<None>();
				}

				ImGui::EndPopup();
			}
			else
			{
				// click outside popup so cancel action
				mState->mAction = createAction<None>();
			}
		}
	}
}
