#pragma once

#include "utility/dllexport.h"
#include "parameter.h"

namespace nap
{
	class ParameterContainer;

	class NAPAPI ParameterGUI
	{
	public:
		ParameterGUI(ParameterService& parameterService);

		void show();

	private:
		void showPresets();
		void showParameters(ParameterContainer& parameterContainer, bool isRoot);
		void HandleLoadPopup();
		void HandleSaveAsPopup();
		bool HandleNewPopup(std::string& outNewFilename);
		void SavePresets();
		void RestorePresets();
		
	private:
		ParameterService&					mParameterService;
		ParameterService::PresetFileList	mPresets;
		ParameterService::PresetFileList	mPrevPresets;
		int									mSelectedPresetIndex = -1;
		int									mPrevSelectedPresetIndex = -1;
	};
}
