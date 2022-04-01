/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>
#include <parameter.h>
#include <parameterservice.h>
#include <parameterguiservice.h>
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	class IMGuiService;

	/**
	 * Shows an ImGUI window that can be used to edit/load/save parameters of a specific group. 
	 * The GUI is created by (recursively) traversing the parameters in a group and 
	 * creating appropriate UI elements for each parameter.
	 *
	 * The editor that is used to create the UI element for a given parameter type can be customized, 
	 * allowing for custom types to be displayed in the UI.  Call nap::ParameterGUIService() to register your own editor. 
	 * Default UI elements are provided for built-in types such as int, float, etc.
	 */
	class NAPAPI ParameterGUI : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		ParameterGUI(nap::Core& core);

		/**
		 * Display all parameters, including parameters in child groups, as UI elements. 
		 * Should be called each frame on update().
		 * When 'newWindow' is set to true a new window will be created.
		 * When 'newWindow' is disabled the parameters will be added to the currently active GUI window.
		 * @param newWindow if the parameters should be added to a new window.
		 */
		void show(bool newWindow = true);

		/**
		 * Load a preset programmatically
		 * @param preset the filename of the preset to load
		 * @param errorState contains the error if the preset load failed
		 * @return if the preset loaded successfully
		 */
		bool load(std::string preset, utility::ErrorState& errorState);

		/**
		 * Returns the index of the currently selected preset if one was selected, returns -1 otherwise
		 * @return the index of the currently selected preset if one was selected, -1 otherwise
		 */
		int getSelectedPresetIndex() const;

		/**
		 * Returns the currently selected preset if one was selected, asserts false otherwise
		 * @return the the currently selected preset if one was selected
		 */
		const std::string& getSelectedPreset() const;

		/**
		 * Initializes the parameter GUI
		 * @param errorState contains the error if initialization failed
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

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
		 * Restore the previously saved preset state
		 */
		void restorePresetState();

		/**
		 * Display all parameters as GUI elements. This function is recursive.
		 * @param parameterGroup the parameter group to display
		 * @param depth the current recursion depth. This is zero by default and is incremented automatically when dealing with nested parameter groups.
		 */
		void showParameters(ParameterGroup& parameterGroup, int depth = 0);

	private:
		ParameterService&							mParameterService;					///< The parameter service
		ParameterGUIService&						mParameterGUIService;				///< The parameter GUI service
		IMGuiService&								mGUIService;						///< The GUI service
		ParameterService::PresetFileList			mPresets;							///< The presets for the currently selected ParameterGroup
		ParameterService::PresetFileList			mPrevPresets;						///< The previous list of presets for the currently selected ParameterGroup. Used to restore the state if the user cancels creation of a new preset.
		int											mSelectedPresetIndex = -1;			///< The currently selected preset's index
		int											mPrevSelectedPresetIndex = -1;		///< The previously selected preset's index. Used to restore the state if the user cancels creation of a new preset.
	};
}
