// Local includes
#include "sequencetrackview.h"
#include "sequenceeditorgui.h"
#include "napcolors.h"

// External Includes
#include <imgui/imgui.h>
#include <iomanip>

namespace nap
{
	std::unordered_map<rttr::type, SequenceTrackViewFactoryFunc>& SequenceTrackView::getFactoryMap()
	{
		static std::unordered_map<rttr::type, SequenceTrackViewFactoryFunc> map;
		return map;
	}

	bool SequenceTrackView::registerFactory(rttr::type type, SequenceTrackViewFactoryFunc func)
	{
		auto& map = getFactoryMap();
		auto it = map.find(type);
		assert(it == map.end()); // duplicate entry
		if (it == map.end())
		{
			map.emplace(type, func);

			return false;
		}

		return false;
	}

	SequenceTrackView::SequenceTrackView(SequenceEditorGUIView& view, SequenceEditorGUIState& state) :
		mView(view), mState(state)
	{

	}


	static bool vector_getter(void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};


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


	void SequenceTrackView::showInspector(const SequenceTrack& track, bool& deleteTrack)
	{
		// begin inspector
		std::ostringstream inspectorIDStream;
		inspectorIDStream << track.mID << "inspector";
		std::string inspectorID = inspectorIDStream.str();

		// manually set the cursor position before drawing new track window
		ImVec2 cursorPos =
			{
				ImGui::GetCursorPosX() ,
				mState.mTrackHeight + 10.0f + ImGui::GetCursorPosY()
			};

		// manually set the cursor position before drawing inspector
		ImVec2 inspectorCursorPos = { cursorPos.x , cursorPos.y };
		ImGui::SetCursorPos(inspectorCursorPos);

		// draw inspector window
		if (ImGui::BeginChild(
			inspectorID.c_str(), // id
			{ mState.mInspectorWidth , mState.mTrackHeight + 5 }, // size
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
				{ windowPos.x + windowSize.x - 5, windowPos.y + mState.mTrackHeight },
				guicolors::black);

			drawList->AddRect(
				windowPos,
				{ windowPos.x + windowSize.x - 5, windowPos.y + mState.mTrackHeight },
				guicolors::white);

			//
			ImVec2 inspectorCursorPos = ImGui::GetCursorPos();
			inspectorCursorPos.x += 5;
			inspectorCursorPos.y += 5;
			ImGui::SetCursorPos(inspectorCursorPos);

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
				deleteTrack = true;
			}

			// pop scale
			ImGui::GetStyle().ScaleAllSizes(1.0f / scale);
		}
		ImGui::EndChild();

		ImGui::SetCursorPos(cursorPos);
	}


	void SequenceTrackView::show(const SequenceTrack& track)
	{
		bool deleteTrack = false;

		showInspector(track, deleteTrack);

		showTrack(track);

		if (deleteTrack)
		{
			auto* controller = getEditor().getControllerWithTrackType(track.get_type());
			assert(controller!= nullptr); // controller not found
			if(controller!= nullptr)
			{
				controller->deleteTrack(track.mID);
				mState.mDirty = true;
			}
		}
	}


	void SequenceTrackView::showTrack(const SequenceTrack& track)
	{
		ImVec2 cursorPos = ImGui::GetCursorPos();

		const ImVec2 windowCursorPos = { cursorPos.x + mState.mInspectorWidth + 5, cursorPos.y };
		ImGui::SetCursorPos(windowCursorPos);

		// begin track
		if (ImGui::BeginChild(
			track.mID.c_str(), // id
			{ mState.mTimelineWidth + 5 , mState.mTrackHeight + 5 }, // size
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
				{ trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + mState.mTrackHeight }, // bottom right position
				guicolors::black); // color

			// draw border of track
			drawList->AddRect(
				trackTopLeft, // top left position
				{ trackTopLeft.x + mState.mTimelineWidth, trackTopLeft.y + mState.mTrackHeight }, // bottom right position
				guicolors::white); // color

			mState.mMouseCursorTime = (mState.mMousePos.x - trackTopLeft.x) / mState.mStepSize;

			showTrackContent(track, trackTopLeft);

			// pop id
			ImGui::PopID();
		}

		ImGui::End();

		//
		ImGui::SetCursorPos({ cursorPos.x, cursorPos.y } );
	}

	const SequencePlayer& SequenceTrackView::getPlayer() { return *mView.mEditor.mSequencePlayer.get(); }

	
	SequenceEditor& SequenceTrackView::getEditor() { return mView.mEditor; }
}