/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
	RTTI_PROPERTY("PresetsDirectory",		&nap::ParameterServiceConfiguration::mPresetsDirectory,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static void gatherParameterGroups(ParameterGroup& group, ParameterService::ParameterGroupList& parameterGroups, int depth = 0)
	{
		parameterGroups.push_back({ &group, depth });
		for (auto& dest_child : group.mChildren)
			gatherParameterGroups(*dest_child, parameterGroups, depth + 1);
	}

	ParameterService::ParameterService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	ParameterService::ParameterGroupList ParameterService::getParameterGroups()
	{
		ParameterGroupList groups;

		if (mRootGroup != nullptr)
			gatherParameterGroups(*mRootGroup, groups);

		return groups;
	}

	ParameterService::PresetFileList ParameterService::getPresets(const ParameterGroup& group) const
	{
		const std::string presetDir = getGroupPresetDirectory(group.mID);

		// Find all files in the preset directory
		std::vector<std::string> files_in_directory;
		utility::listDir(presetDir.c_str(), files_in_directory);

		PresetFileList presets;
		for (const auto& filename : files_in_directory)
		{
			// Ignore directories
			if (utility::dirExists(filename))
				continue;

			if (utility::getFileExtension(filename) == "json")
			{
				presets.push_back(utility::getFileName(filename));
			}
		}

		// Sort
		std::sort(presets.begin(), presets.end(), [](std::string a, std::string b)
		{
			return a < b;
		});

		return presets;
	}


	std::string ParameterService::getPresetPath(const std::string& groupID, const std::string& filename) const
	{
		std::string preset_path = getGroupPresetDirectory(groupID);
		if (preset_path.back() != '/' && preset_path.back() != '\\')
			preset_path += "/";

		preset_path += filename;
		std::transform(preset_path.begin(), preset_path.end(), preset_path.begin(), ::tolower);

		return preset_path;
	}


	std::string ParameterService::getGroupPresetDirectory(const std::string& groupID) const
	{
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		std::string group_directory = configuration->mPresetsDirectory;
		if (group_directory.back() != '/' && group_directory.back() != '\\')
			group_directory += "/";

		group_directory += groupID;
		std::transform(group_directory.begin(), group_directory.end(), group_directory.begin(), ::tolower);

		return group_directory;
	}


	bool ParameterService::loadPreset(ParameterGroup& group, const std::string& presetFile, utility::ErrorState& errorState)
	{
		std::string preset_path = getPresetPath(group.mID, presetFile);

		// Load the parameters from the preset
		rtti::DeserializeResult deserialize_result;
		if (!rtti::deserializeJSONFile(preset_path, rtti::EPropertyValidationMode::DisallowMissingProperties, rtti::EPointerPropertyMode::NoRawPointers, getCore().getResourceManager()->getFactory(), deserialize_result, errorState))
			return false;

		// Resolve links
		if (!rtti::DefaultLinkResolver::sResolveLinks(deserialize_result.mReadObjects, deserialize_result.mUnresolvedPointers, errorState))
			return false;

		// Find the root parameter group in the preset file and apply parameters
		for (auto& object : deserialize_result.mReadObjects)
		{
			if (object->get_type().is_derived_from<ParameterGroup>() && object->mID == group.mID)
			{
				setParametersRecursive(*rtti_cast<ParameterGroup>(object.get()), group);
				presetLoaded();
				return true;
			}
		}

		errorState.fail("The preset file %s does not contain a ParameterGroup with name %s", presetFile.c_str(), group.mID.c_str());
		return false;
	}


	bool ParameterService::savePreset(ParameterGroup& group, const std::string& presetFile, utility::ErrorState& errorState)
	{
		// Ensure the presets directory exists
		utility::makeDirs(utility::getAbsolutePath(getGroupPresetDirectory(group.mID)));

		std::string preset_path = getPresetPath(group.mID, presetFile);

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects({ &group }, writer, errorState))
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


	void ParameterService::postResourcesLoaded()
	{
		// Whenever the main json is (re)loaded, update the root parameter group
		// We search for the root parameter group by finding the group that has no parent
		using ParentMap = std::unordered_map<const ParameterGroup*, const ParameterGroup*>;
		ParentMap parent_map;
		std::vector<rtti::ObjectPtr<ParameterGroup>> parameter_groups = getCore().getResourceManager()->getObjects<ParameterGroup>();
		for (auto& parameter_group : parameter_groups)
			for (auto& child_group : parameter_group->mChildren)
				parent_map[child_group.get()] = parameter_group.get();

		mRootGroup = nullptr;
		for (auto& parameter_group : parameter_groups)
		{
			if (parent_map.find(parameter_group.get()) == parent_map.end())
			{
				mRootGroup = parameter_group.get();
				break;
			}
		}

		fileLoaded();
	}


	void ParameterService::setParametersRecursive(const ParameterGroup& sourceParameters, ParameterGroup& destinationParameters)
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

		// Recursively apply the parameters of all child groups
		for (auto& dest_child : destinationParameters.mChildren)
		{
			const ResourcePtr<ParameterGroup>& source_child = sourceParameters.findChild(dest_child->mID);
			if (source_child != nullptr)
				setParametersRecursive(*source_child, *dest_child);
		}
	}
}
