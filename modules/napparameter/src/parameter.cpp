#include "parameter.h"
#include <nap/core.h>
#include "rtti/jsonreader.h"
#include "rtti/defaultlinkresolver.h"
#include "rtti/jsonwriter.h"
#include <fstream>
#include "utility/fileutils.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ParameterContainer)
	RTTI_PROPERTY("Parameters",	&nap::ParameterContainer::mParameters, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Children",	&nap::ParameterContainer::mChildren, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


RTTI_BEGIN_CLASS(nap::ParameterFloat)
	RTTI_PROPERTY("Value",		&nap::ParameterFloat::mValue,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Minimum",	&nap::ParameterFloat::mMinimum,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Maximum",	&nap::ParameterFloat::mMaximum,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	ResourcePtr<Parameter> ParameterContainer::findParameter(const std::string& name) const
	{
		for (auto& param : mParameters)
			if (param->mID == name)
				return param;

		return nullptr;
	}


	ResourcePtr<ParameterContainer> ParameterContainer::findChild(const std::string& name) const
	{
		for (auto& param : mChildren)
			if (param->mID == name)
				return param;

		return nullptr;
	}

	ParameterService::ParameterService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	ParameterService::PresetFileList ParameterService::getPresets() const
	{
		// Find all files in the specified directory
		std::vector<std::string> files_in_directory;
		utility::listDir("presets", files_in_directory);

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


	bool ParameterService::loadPreset(const std::string& presetFile, utility::ErrorState& errorState)
	{
		std::string preset_path = "presets/" + presetFile;

		rtti::DeserializeResult deserialize_result;
		if (!rtti::readJSONFile(preset_path, rtti::EPropertyValidationMode::DisallowMissingProperties, getCore().getResourceManager()->getFactory(), deserialize_result, errorState))
			return false;

		if (!rtti::DefaultLinkResolver::sResolveLinks(deserialize_result.mReadObjects, deserialize_result.mUnresolvedPointers, errorState))
			return false;
		
		for (auto& object : deserialize_result.mReadObjects)
		{
			if (object->get_type().is_derived_from<ParameterContainer>())
			{
				setParametersRecursive(*rtti_cast<ParameterContainer>(object.get()), *mRootContainer);
				return true;
			}
		}

		return false;
	}


	bool ParameterService::savePreset(const std::string& presetFile, utility::ErrorState& errorState)
	{
		utility::makeDirs(utility::getAbsolutePath("presets"));
		std::string preset_path = "presets/" + presetFile;

		rtti::JSONWriter writer;
		if (!rtti::serializeObjects({ mRootContainer.get() }, writer, errorState))
			return false;

		std::ofstream output(preset_path, std::ios::binary | std::ios::out);
		if (!errorState.check(output.is_open() && output.good(), "Failed to open %s for writing", preset_path.c_str()))
			return false;

		std::string json = writer.GetJSON();
		output.write(json.data(), json.size());

		return true;
	}


	void ParameterService::resourcesLoaded()
	{
		ResourcePtr<ParameterContainer> root_params = getCore().getResourceManager()->findObject<ParameterContainer>("Parameters");
		assert(root_params != nullptr);
		
		mRootContainer = root_params;
	}


	void ParameterService::setParametersRecursive(const ParameterContainer& sourceParameters, ParameterContainer& destinationParameters)
	{
		for (auto& param : destinationParameters.mParameters)
		{
			const ResourcePtr<Parameter>& new_param = sourceParameters.findParameter(param->mID);
			if (new_param != nullptr)
				param->setValue(*new_param);
		}

		for (auto& dest_child : destinationParameters.mChildren)
		{
			const ResourcePtr<ParameterContainer>& source_child = sourceParameters.findChild(dest_child->mID);
			if (source_child != nullptr)
				setParametersRecursive(*source_child, *dest_child);
		}
	}
}