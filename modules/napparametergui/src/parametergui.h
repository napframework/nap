/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <parameter.h>
#include <parameterservice.h>

namespace nap
{
	class ParameterGroup;

	/**
	 * The ParameterGUI class can be used to easily show an ImGUI window which can be used to edit/load/save parameter presets. The GUI maintains all needed state.
	 * The GUI is built up by automatically (and recursively) traversing the parameters in a group and creating appropriate UI elements for each parameter.
	 *
	 * The function that is used to create the UI element for a given parameter type can be customized, thus allowing for custom types to be displayed in the UI.
	 * Default UI elements are provided for built-in types such as int, float, etc.
	 */
	class NAPAPI ParameterGUI
	{
	public:
		/**
		 * A registered CreateParameterEditor function is called whenever the UI for a particular Parameter type needs to be shown. The parameter in question is passed
		 * as argument to the provided function.
		 *
		 * The provided function should use ImGUI internally to draw the UI for that parameter. It is important to note that the value should always be set
		 * on the parameter through the setValue function and not by editing the mValue member directly. The reason for this is that, when editing mValue directly,
		 * no signals will be raised, so clients of the Parameter that are watching for value changes will not receive any.
		 * To ensure this works as expected, the general pattern of a parameter editor looks something like the following:
		 * 
		 * 	float value = parameter.mValue;
		 *	if (ImGui::SliderFloat(parameter.getDisplayName().c_str(), &value, parameter.mMinimum, parameter.mMaximum))
		 *		parameter.setValue(value);
		 * 
		 * Note that we first retrieve a copy of the old value and display an ImGUI widget that works on that local value. After the user makes an edit,
		 * we set the value on the parameter based on the local copy, which now contains the new value.
		 */
		using CreateParameterEditor = std::function<void(Parameter&)>;

		ParameterGUI(ParameterService& parameterService, ParameterGroup& group);

		/**
		 * Main function to render a group of parameter, including nested groups. Should be called each frame.
		 * When 'newWindow' is set to true (default) a new window will be created.
		 * When 'newWindow' is disabled the parameters will be added to the currently active GUI window.
		 * @param parameterGroup the group of parameters to show. 
		 * @param newWindow if the parameters should be added to a new window.
		 */
		void show(bool newWindow = true);

		/**
		 * Register an editor creation function for the given type. The editor creation function is invoked whenever a parameter of the given type 
		 * is encountered and should use ImGUI internally to draw the UI for that parameter. The parameter in question is passed to the creation func.
		 *
		 * Registering a creation function for a type that was previously registered will overwrite the previous function, which allows you to customize 
		 * the behavior for the built-in types that are registered by default
		 *
		 * @param type The type to register an editor creation function for
		 * @param createParameterEditorFunc The editor creation function
		 */
		void registerParameterEditor(const rtti::TypeInfo& type, const CreateParameterEditor& createParameterEditorFunc);

	private:
		/**
		 * Render the preset selection/save/load UI. 
		 */
		void showPresets(const ParameterGroup* parameterGroup);

		/**
		 * Render the parameter UI for a specific group
		 *
		 * @param parameterGroup The group to draw the parameters fro
		 */
		void showParameters(ParameterGroup& parameterGroup);

		/**
		 * Show and handle the UI to load presets
		 */
		void handleLoadPresetPopup();

		/**
		 * Show and handle the UI to save presets
		 */
		void handleSaveAsPresetPopup();

		/**
		 * Show and handle the UI to create new presets
		 */
		bool handleNewPresetPopup(std::string& outNewFilename);

		/**
		 * Save the preset state so that we can restore it to what it was if the user cancels creation
		 */
		void savePresetState();

		/**
		 * Restore the preveiously saved preset state
		 */
		void restorePresetState();

		/**
		 * Register default editor creation functions for all built-in types
		 */
		void registerDefaultParameterEditors();

	private:
		using ParameterEditorMap = std::unordered_map<rtti::TypeInfo, CreateParameterEditor>;

		ParameterService&							mParameterService;					///< The parameter service
		ParameterEditorMap							mParameterEditors;					///< The editor function to use per parameter type
		ParameterGroup*								mParameterGroup;					///< All available ParameterGroups
		ParameterService::PresetFileList			mPresets;							///< The presets for the currently selected ParameterGroup
		ParameterService::PresetFileList			mPrevPresets;						///< The previous list of presets for the currently selected ParameterGroup. Used to restore the state if the user cancels creation of a new preset.
		int											mSelectedPresetIndex = -1;			///< The currently selected preset's index
		int											mPrevSelectedPresetIndex = -1;		///< The previously selected preset's index. Used to restore the state if the user cancels creation of a new preset.
	};
}
