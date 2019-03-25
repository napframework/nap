#pragma once

// External Includes
#include "nap/service.h"
#include "nap/resourceptr.h"

namespace nap
{
	class ParameterContainer;

	class NAPAPI ParameterService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		using PresetFileList = std::vector<std::string>;
		ParameterService(ServiceConfiguration* configuration);

		PresetFileList getPresets() const;
		bool loadPreset(const std::string& presetFile, utility::ErrorState& errorState);
		bool savePreset(const std::string& presetFile, utility::ErrorState& errorState);
	
		bool hasParameters() const { return mRootContainer != nullptr; }
		ParameterContainer& getParameters() { assert(hasParameters()); return *mRootContainer; }

	protected:		
		virtual void resourcesLoaded() override;
		std::string getPresetPath(const std::string& filename) const;

	private:
		void setParametersRecursive(const ParameterContainer& sourceParameters, ParameterContainer& destinationParameters);

	private:
		ResourcePtr<ParameterContainer> mRootContainer;
	};


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
		std::string mRootParameterContainer = "Parameters";
		std::string mPresetsDirectory = "Presets";
	};
}
