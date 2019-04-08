#include <parameterservice.h>
#include <parameter.h>
#include <nap/core.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/jsonwriter.h>
#include <utility/fileutils.h>
#include <fstream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ParameterServiceConfiguration)
	RTTI_PROPERTY("RootParameterContainer",		&nap::ParameterServiceConfiguration::mRootParameterContainer,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PresetsDirectory",			&nap::ParameterServiceConfiguration::mPresetsDirectory,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static void gatherParameterContainers(ParameterContainer& container, ParameterService::ParameterContainerList& parameterContainers)
	{
		parameterContainers.push_back(&container);
		for (auto& dest_child : container.mChildren)
			gatherParameterContainers(*dest_child, parameterContainers);
	}

	ParameterService::ParameterService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	ParameterService::ParameterContainerList ParameterService::getParameterContainers()
	{
		ParameterContainerList containers;

		if (mRootContainer != nullptr)
			gatherParameterContainers(*mRootContainer, containers);

		return containers;
	}

	ParameterService::PresetFileList ParameterService::getPresets(const ParameterContainer& container) const
	{
		const std::string presetDir = getContainerPresetDirectory(container.mID);

		// Find all files in the preset directory
		std::vector<std::string> files_in_directory;
		utility::listDir(presetDir.c_str(), files_in_directory);

		PresetFileList presets;
		for (const auto& filename : files_in_directory)
		{
			rtti::TypeInfo service = rtti::TypeInfo::empty();

			// Ignore directories
			if (utility::dirExists(filename))
				continue;

			if (utility::getFileExtension(filename) == "json")
			{
				presets.push_back(utility::getFileName(filename));
			}
		}

		return presets;
	}


	std::string ParameterService::getPresetPath(const std::string& containerID, const std::string& filename) const
	{
		std::string preset_path = getContainerPresetDirectory(containerID);
		if (preset_path.back() != '/' && preset_path.back() != '\\')
			preset_path += "/";

		preset_path += filename;
		return preset_path;
	}


	std::string ParameterService::getContainerPresetDirectory(const std::string& containerID) const
	{
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		std::string container_directory = configuration->mPresetsDirectory;
		if (container_directory.back() != '/' && container_directory.back() != '\\')
			container_directory += "/";

		container_directory += containerID;
		return container_directory;
	}

	bool ParameterService::loadPreset(ParameterContainer& container, const std::string& presetFile, utility::ErrorState& errorState)
	{
		std::string preset_path = getPresetPath(container.mID, presetFile);

		// Load the parameters from the preset
		rtti::DeserializeResult deserialize_result;
		if (!rtti::readJSONFile(preset_path, rtti::EPropertyValidationMode::DisallowMissingProperties, getCore().getResourceManager()->getFactory(), deserialize_result, errorState))
			return false;

		// Resolve links
		if (!rtti::DefaultLinkResolver::sResolveLinks(deserialize_result.mReadObjects, deserialize_result.mUnresolvedPointers, errorState))
			return false;

		// Find the root parameter container in the preset file and apply parameters
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();
		for (auto& object : deserialize_result.mReadObjects)
		{
			if (object->get_type().is_derived_from<ParameterContainer>() && object->mID == container.mID)
			{
				setParametersRecursive(*rtti_cast<ParameterContainer>(object.get()), container);
				return true;
			}
		}

		errorState.fail("The preset file %s does not contain a ParameterContainer with name %s", presetFile.c_str(), configuration->mRootParameterContainer.c_str());
		return false;
	}


	bool ParameterService::savePreset(ParameterContainer& container, const std::string& presetFile, utility::ErrorState& errorState)
	{
		// Ensure the presets directory exists
		utility::makeDirs(utility::getAbsolutePath(getContainerPresetDirectory(container.mID)));

		std::string preset_path = getPresetPath(container.mID, presetFile);

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects({ &container }, writer, errorState))
			return false;

		// Open output file
		std::ofstream output(preset_path, std::ios::binary | std::ios::out);
		if (!errorState.check(output.is_open() && output.good(), "Failed to open %s for writing", preset_path.c_str()))
			return false;

		// Write to disk
		std::string json = writer.GetJSON();
		output.write(json.data(), json.size());

		return true;
	}


	void ParameterService::resourcesLoaded()
	{
		// Whenever the main json is (re)loaded, update the root parameter container
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();
		ResourcePtr<ParameterContainer> root_params = getCore().getResourceManager()->findObject<ParameterContainer>(configuration->mRootParameterContainer);
		mRootContainer = root_params;
	}


	void ParameterService::setParametersRecursive(const ParameterContainer& sourceParameters, ParameterContainer& destinationParameters)
	{
		// Apply all parameters in the source to the destination. 
		// Note that it's not considered an error if a parameter is preset in the source, but not in the destination. 
		// This is to ensure that changes in the parameter structure for a project don't invalidate entire presets; they just invalidate the changed parts.
		for (auto& param : destinationParameters.mParameters)
		{
			const ResourcePtr<Parameter>& new_param = sourceParameters.findParameter(param->mID);
			if (new_param != nullptr)
				param->setValue(*new_param);
		}

		// Recursively apply the parameters of all child containers
		for (auto& dest_child : destinationParameters.mChildren)
		{
			const ResourcePtr<ParameterContainer>& source_child = sourceParameters.findChild(dest_child->mID);
			if (source_child != nullptr)
				setParametersRecursive(*source_child, *dest_child);
		}
	}
}