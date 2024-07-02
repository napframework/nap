/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include <parameterservice.h>

// External Includes
#include <nap/core.h>
#include <rtti/jsonreader.h>
#include <rtti/defaultlinkresolver.h>
#include <rtti/jsonwriter.h>
#include <utility/fileutils.h>
#include <fstream>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterService, "Global parameter settings")
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ParameterServiceConfiguration)
	RTTI_PROPERTY("PresetsDirectory",		&nap::ParameterServiceConfiguration::mPresetsDirectory,		nap::rtti::EPropertyMetaData::Default, "Directory to store/load presets")
RTTI_END_CLASS

namespace nap
{
	ParameterService::ParameterService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


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
				// set parameters & notify listeners
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

		// Create parameter group and copy group member properties
		// Don't serialize the given group, which might contain properties we're not interested in
		auto ser_group = std::make_unique<nap::ParameterGroup>();
		auto properties = ser_group->get_type().get_properties();
		for (const rtti::Property& property : properties)
		{
			rtti::Variant new_value = property.get_value(group);
			bool success = property.set_value(*ser_group, new_value);
			if (!errorState.check(success, "Failed to copy property: %s", property.get_name().data()))
				return false;
		}

		// Serialize current set of parameters to json
		rtti::JSONWriter writer;
		if (!rtti::serializeObjects({ ser_group.get() }, writer, errorState))
			return false;

		// Open output file
		std::string preset_path = getPresetPath(group.mID, presetFile);
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
		fileLoaded();
	}


	void ParameterService::setParametersRecursive(const ParameterGroup& sourceGroup, ParameterGroup& destinationGroup)
	{
		// Apply all parameters in the source to the destination. 
		// Note that it's not considered an error if a parameter is preset in the source, but not in the destination. 
		// This is to ensure that changes in the parameter structure for a project don't invalidate entire presets; they just invalidate the changed parts.
		for (auto& dst_member : destinationGroup.mMembers)
		{
			auto src_it = std::find_if(sourceGroup.mMembers.begin(), sourceGroup.mMembers.end(), [&dst_member](const auto& src_member)
				{
					return src_member->mID == dst_member->mID;
				});

			if (src_it != sourceGroup.mMembers.end())
			{
				dst_member->setValue(**src_it);
			}
		}

		// Recursively apply the parameters of all child groups
		for (auto& dst_child : destinationGroup.mChildren)
		{
			auto src_it = std::find_if(sourceGroup.mChildren.begin(), sourceGroup.mChildren.end(), [&dst_child](const auto& src_child)
				{
					return src_child->mID == dst_child->mID;
				});

			if (src_it != sourceGroup.mChildren.end())
			{
				setParametersRecursive(**src_it, *dst_child);
			}
		}
	}
}
