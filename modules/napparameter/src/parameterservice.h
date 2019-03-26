#pragma once

// External Includes
#include <nap/service.h>
#include <nap/resourceptr.h>

namespace nap
{
	class ParameterContainer;

	/**
	 * The ParameterService manages the Parameters for a project. It provides support for loading/saving presets of Parameters
	 */
	class NAPAPI ParameterService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using PresetFileList = std::vector<std::string>;
		ParameterService(ServiceConfiguration* configuration);

		/**
		 * Get a list of all available preset files
		 *
		 * @return The list of presets
		 */
		PresetFileList getPresets() const;

		/**
		 * Load a preset from the specified file. The parameters in the Preset will be automatically applied.
		 * The list of available presets files can be retrieved through getPresets()
		 *
		 * @param presetFile The path to the the preset file to load
		 * @param errorState Detailed error information when load fails
		 *
		 * @return Whether the load of the preset failed or succeeded
		 */
		bool loadPreset(const std::string& presetFile, utility::ErrorState& errorState);

		/**
		 * Save the current set of parameters/values to the specified file as a preset.
		 *
		 * @param presetFile The path to save to
		 * @param errorState Detailed error information when save fails
		 *
		 * @return Whether the save of the preset failed or succeeded
		 */
		bool savePreset(const std::string& presetFile, utility::ErrorState& errorState);
	
		/**
		 * Checks whether any parameters have been defined for the current project
		 *
		 * @return whether any parameters have been defined for the current project
		 */
		bool hasParameters() const { return mRootContainer != nullptr; }

		/**
		 * Get the parameters defined for this project.
		 *
		 * @return The parameters defined for this project.
		 */
		ParameterContainer& getParameters() { assert(hasParameters()); return *mRootContainer; }

	protected:

		/**
		 * Called when a json file has been (re)loaded. Used to re-apply the presets.
		 */
		virtual void resourcesLoaded() override;

		/**
		 * Get the full path to the preset file
		 *
		 * @param filename The preset file to get the full path of
		 *
		 * @return Full path to the preset file
		 */
		std::string getPresetPath(const std::string& filename) const;

	private:

		/** 
		 * Helper function to recursively apply parameters from a preset to the current active set of parameters
		 *
		 * @param sourceParameters The parameter container that contains the parameters which should be applied
		 * @param destinationParameters The parameter container that contains the destination parameters which should be overwritten with data from the preset
		 */
		void setParametersRecursive(const ParameterContainer& sourceParameters, ParameterContainer& destinationParameters);

	private:
		ResourcePtr<ParameterContainer> mRootContainer;		///< The root parameter container containing the parameters for this project
	};


	/**
	 * The ParameterServiceConfiguration is used to configure the ParameterService. 
	 * It allows the user to specify the id of the 'root' parameter container and the directory where presets should be saved to.
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
		std::string mRootParameterContainer = "Parameters";	///< Property: 'RootParameterContainer' The ID of the root parameter container
		std::string mPresetsDirectory		= "Presets";	///< Property: 'PresetsDirectory' The directory where presets should be saved to/loaded from
	};
}
