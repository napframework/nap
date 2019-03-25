#include "parameter.h"
#include <nap/core.h>
#include "rtti/jsonreader.h"
#include "rtti/defaultlinkresolver.h"
#include "rtti/jsonwriter.h"
#include <fstream>
#include "utility/fileutils.h"

#define DEFINE_NUMERIC_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,		nap::rtti::EPropertyMetaData::Default)					\
		RTTI_PROPERTY("Minimum",	&Type::mMinimum,		nap::rtti::EPropertyMetaData::Default)				\
		RTTI_PROPERTY("Maximum",	&Type::mMaximum,		nap::rtti::EPropertyMetaData::Default)				\
	RTTI_END_CLASS

#define DEFINE_SIMPLE_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,		nap::rtti::EPropertyMetaData::Default)					\
	RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ParameterContainer)
	RTTI_PROPERTY("Parameters",	&nap::ParameterContainer::mParameters, nap::rtti::EPropertyMetaData::Embedded)
	RTTI_PROPERTY("Children",	&nap::ParameterContainer::mChildren, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ParameterServiceConfiguration)
	RTTI_PROPERTY("RootParameterContainer",		&nap::ParameterServiceConfiguration::mRootParameterContainer,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PresetsDirectory",			&nap::ParameterServiceConfiguration::mPresetsDirectory,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterEnumBase)
RTTI_END_CLASS

DEFINE_NUMERIC_PARAMETER(nap::ParameterFloat)
DEFINE_NUMERIC_PARAMETER(nap::ParameterDouble)
DEFINE_NUMERIC_PARAMETER(nap::ParameterLong)
DEFINE_NUMERIC_PARAMETER(nap::ParameterInt)
DEFINE_NUMERIC_PARAMETER(nap::ParameterChar)
DEFINE_NUMERIC_PARAMETER(nap::ParameterByte)

DEFINE_NUMERIC_PARAMETER(nap::ParameterVec2)
DEFINE_NUMERIC_PARAMETER(nap::ParameterIVec2)
DEFINE_NUMERIC_PARAMETER(nap::ParameterVec3)

DEFINE_SIMPLE_PARAMETER(nap::ParameterBool)
DEFINE_SIMPLE_PARAMETER(nap::ParameterRGBColorFloat)

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
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		// Find all files in the specified directory
		std::vector<std::string> files_in_directory;
		utility::listDir(configuration->mPresetsDirectory.c_str(), files_in_directory);

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


	std::string ParameterService::getPresetPath(const std::string& filename) const
	{
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		std::string preset_path = configuration->mPresetsDirectory;
		if (preset_path.back() != '/' && preset_path.back() != '\\')
			preset_path += "/";

		preset_path += filename;
		return preset_path;
	}


	bool ParameterService::loadPreset(const std::string& presetFile, utility::ErrorState& errorState)
	{
		std::string preset_path = getPresetPath(presetFile);

		if (!errorState.check(mRootContainer != nullptr, "There is no root parameter container active in the project. Ensure the ParameterServiceConfiguration is correct"))
			return false;

		rtti::DeserializeResult deserialize_result;
		if (!rtti::readJSONFile(preset_path, rtti::EPropertyValidationMode::DisallowMissingProperties, getCore().getResourceManager()->getFactory(), deserialize_result, errorState))
			return false;

		if (!rtti::DefaultLinkResolver::sResolveLinks(deserialize_result.mReadObjects, deserialize_result.mUnresolvedPointers, errorState))
			return false;

		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		for (auto& object : deserialize_result.mReadObjects)
		{
			if (object->get_type().is_derived_from<ParameterContainer>() && object->mID == configuration->mRootParameterContainer)
			{
				setParametersRecursive(*rtti_cast<ParameterContainer>(object.get()), *mRootContainer);
				return true;
			}
		}

		return false;
	}


	bool ParameterService::savePreset(const std::string& presetFile, utility::ErrorState& errorState)
	{
		if (!errorState.check(mRootContainer != nullptr, "There is no root parameter container active in the project. Ensure the ParameterServiceConfiguration is correct"))
			return false;

		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		utility::makeDirs(utility::getAbsolutePath(configuration->mPresetsDirectory));
		std::string preset_path = getPresetPath(presetFile);

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
		const ParameterServiceConfiguration* configuration = getConfiguration<ParameterServiceConfiguration>();

		ResourcePtr<ParameterContainer> root_params = getCore().getResourceManager()->findObject<ParameterContainer>(configuration->mRootParameterContainer);
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