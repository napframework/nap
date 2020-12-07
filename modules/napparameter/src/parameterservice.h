/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>

namespace nap
{
	class ParameterGroup;

	/**
	 * The ParameterService manages the Parameters for a project. It provides support for loading/saving presets of Parameters
	 */
	class NAPAPI ParameterService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		/**
		 * This struct holds additional info on top of a group, most importantly the depth of the group within the full tree
		 */
		struct ParameterGroupInfo
		{
			ResourcePtr<ParameterGroup> mGroup;		///< The group we're storing info for
			int							mDepth;		///< The depth of this group within the full group tree
		};

		using PresetFileList = std::vector<std::string>;
		using ParameterGroupList = std::vector<ParameterGroupInfo>;
		ParameterService(ServiceConfiguration* configuration);

		/**
		 * Get all parameter groups
		 * @return The list of all parameter groups
		 */
		ParameterGroupList getParameterGroups();

		/**
		 * Get a list of all available preset files for the specified group
		 *
		 * @return The list of presets
		 */
		PresetFileList getPresets(const ParameterGroup& group) const;

		/**
		 * Load a preset from the specified file. The parameters in the Preset will be automatically applied.
		 * The list of available presets files can be retrieved through getPresets()
		 *
		 * @param group the parameter group.
		 * @param presetFile The path to the the preset file to load
		 * @param errorState Detailed error information when load fails
		 *
		 * @return Whether the load of the preset failed or succeeded
		 */
		bool loadPreset(ParameterGroup& group, const std::string& presetFile, utility::ErrorState& errorState);

		/**
		 * Save the current set of parameters/values to the specified file as a preset.
		 *
		 * @param group the parameter group.
		 * @param presetFile The path to save to
		 * @param errorState Detailed error information when save fails
		 *
		 * @return Whether the save of the preset failed or succeeded
		 */
		bool savePreset(ParameterGroup& group, const std::string& presetFile, utility::ErrorState& errorState);

		/**
		 * Get the full path to the preset file
		 * @param groupID The group id
		 * @param filename The preset file to get the full path of
		 * @return Full path to the preset file
		 */
		std::string getPresetPath(const std::string& groupID, const std::string& filename) const;

		/**
		 * Get the directory that presets for the specified group are stored in
		 *
		 * @param groupID The group id
		 * @return The directory that presets for the specified group are stored in
		 */
		std::string getGroupPresetDirectory(const std::string& groupID) const;

		/**
         * Signal that is emitted when a preset is loaded
         */
		nap::Signal<> presetLoaded;


		/**
		 * Signal that is emitted when a file reload occurs
		 */
		nap::Signal<> fileLoaded;

	protected:
		/**
		 * Called when a json file has been (re)loaded. Used to re-apply the presets.
		 */
		virtual void postResourcesLoaded() override;

	private:

		/** 
		 * Helper function to recursively apply parameters from a preset to the current active set of parameters
		 *
		 * @param sourceParameters The parameter group that contains the parameters which should be applied
		 * @param destinationParameters The parameter group that contains the destination parameters which should be overwritten with data from the preset
		 */
		void setParametersRecursive(const ParameterGroup& sourceParameters, ParameterGroup& destinationParameters);
	};


	/**
	 * The ParameterServiceConfiguration is used to configure the ParameterService. 
	 * It allows the user to specify the id of the 'root' parameter group and the directory where presets should be saved to.
	 */
	class NAPAPI ParameterServiceConfiguration : public ServiceConfiguration
	{
		RTTI_ENABLE(ServiceConfiguration)
	public:

		/**
		 *	@return The Service type associated with this configuration.
		 */
		virtual rtti::TypeInfo getServiceType()
		{
			return RTTI_OF(ParameterService);
		}

	public:
		std::string mPresetsDirectory		= "Presets";	///< Property: 'PresetsDirectory' The directory where presets should be saved to/loaded from
	};
}
