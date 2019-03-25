#pragma once

#include "utility/dllexport.h"
#include "parameter.h"
#include "parameterservice.h"

namespace nap
{
	class ParameterContainer;

	class NAPAPI ParameterGUI
	{
	public:
		using CreateParameterEditor = std::function<void(Parameter&)>;

		ParameterGUI(ParameterService& parameterService);

		void show();
		void registerParameterEditor(const rtti::TypeInfo& type, const CreateParameterEditor& createParameterEditorFunc);

	private:
		void showPresets();
		void showParameters(ParameterContainer& parameterContainer, bool isRoot);
		void handleLoadPopup();
		void handleSaveAsPopup();
		bool handleNewPopup(std::string& outNewFilename);
		void savePresets();
		void restorePresets();

		void registerDefaultParameterEditors();
	private:
		using ParameterEditorMap = std::unordered_map<rtti::TypeInfo, CreateParameterEditor>;

		ParameterService&					mParameterService;
		ParameterEditorMap					mParameterEditors;
		ParameterService::PresetFileList	mPresets;
		ParameterService::PresetFileList	mPrevPresets;
		int									mSelectedPresetIndex = -1;
		int									mPrevSelectedPresetIndex = -1;
	};
}
