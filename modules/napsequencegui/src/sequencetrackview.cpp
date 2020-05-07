// Local includes
#include "sequencetrackview.h"
#include "sequenceeditorgui.h"

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


	const SequencePlayer& SequenceTrackView::getPlayer() { return *mView.mEditor.mSequencePlayer.get(); }

	
	SequenceEditor& SequenceTrackView::getEditor() { return mView.mEditor; }
}