/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <parameter.h>
#include <parameterservice.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	/**
	 * Used to show an ImGUI window which can be used to edit/load/save parameter presets. The GUI maintains all needed state.
	 * The GUI is built up by automatically (and recursively) traversing the parameters in a group and creating appropriate UI elements for each parameter.
	 *
	 * The function that is used to create the UI element for a given parameter type can be customized, thus allowing for custom types to be displayed in the UI.
	 * Default UI elements are provided for built-in types such as int, float, etc.
	 */
	class NAPAPI ParameterGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ParameterGUI(nap::Core& core);

		/**
		 * Main function to render the group of parameters, including nested groups. Should be called each frame.
		 * When 'newWindow' is set to true (default) a new window will be created.
		 * When 'newWindow' is disabled the parameters will be added to the currently active GUI window.
		 * @param newWindow if the parameters should be added to a new window.
		 */
		void show(bool newWindow = true);

		/**
		 * Initializes the parameter GUI
		 * @param errorState contains the error if initialization failed
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

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
		static bool registerParameterEditor(const rtti::TypeInfo& type, const CreateParameterEditor& createParameterEditorFunc);

		bool mSerializable = true;										///< Property: 'Serializable' if the group, including child groups, is a preset that can be saved and loaded using this GUI.
		ResourcePtr<nap::ParameterGroup> mParameterGroup = nullptr;		///< Property: 'Group' Which parameter group to show, including all child groups.

	private:
		/**
		 * Render the preset selection/save/load UI. 
		 */
		void showPresets();

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

	private:
		ParameterService&							mParameterService;					///< The parameter service
		ParameterService::PresetFileList			mPresets;							///< The presets for the currently selected ParameterGroup
		ParameterService::PresetFileList			mPrevPresets;						///< The previous list of presets for the currently selected ParameterGroup. Used to restore the state if the user cancels creation of a new preset.
		int											mSelectedPresetIndex = -1;			///< The currently selected preset's index
		int											mPrevSelectedPresetIndex = -1;		///< The previously selected preset's index. Used to restore the state if the user cancels creation of a new preset.
	};
}
