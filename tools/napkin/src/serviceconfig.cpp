// Local Includes
#include "serviceconfig.h"
#include "naputils.h"

// External Includes
#include <nap/logger.h>
#include <rtti/jsonwriter.h>
#include <rtti/jsonreader.h>
#include <mathutils.h>
#include <qdir.h>
#include <qstring.h>

namespace napkin
{
	ServiceConfig::ServiceConfig(nap::Core& core) : QObject(), mCore(core)
	{
		copyServiceConfig();
	}


	const QString& ServiceConfig::getFilename() const
	{
		return mCurrentConfigFilename;
	}


	void ServiceConfig::newServiceConfig()
	{
		// Create a clean (default) copy for every loaded service config
		std::vector<std::unique_ptr<nap::ServiceConfiguration>> new_defaults;
		new_defaults.reserve(mServiceConfigs.size());
		for (const auto& config : mServiceConfigs)
		{
			new_defaults.emplace_back(config->get_type().create<nap::ServiceConfiguration>());
		}
		mServiceConfigs = std::move(new_defaults);
		mCurrentConfigFilename.clear();
		nap::Logger::info("Created new default configuration");
	}


	void ServiceConfig::loadServiceConfig(QString serviceConfigFile)
	{
		// De-serialize file
		assert(mCore.isInitialized());
		nap::rtti::DeserializeResult result;
		nap::utility::ErrorState error;

		if (!nap::rtti::deserializeJSONFile(serviceConfigFile.toStdString(),
			nap::rtti::EPropertyValidationMode::DisallowMissingProperties,
			nap::rtti::EPointerPropertyMode::OnlyRawPointers,
			mCore.getResourceManager()->getFactory(),
			result,
			error))
		{
			nap::Logger::fatal(error.toString());
			return;
		}

		// Clear and fill
		mServiceConfigs.clear();
		std::set<nap::rtti::TypeInfo> types;
		for (std::unique_ptr<nap::rtti::Object>& object : result.mReadObjects)
		{
			// Check if it's indeed a service configuration object
			nap::rtti::TypeInfo object_type = object->get_type();
			std::unique_ptr<nap::ServiceConfiguration> config = rtti_cast<nap::ServiceConfiguration>(object);
			if (config == nullptr)
			{
				nap::Logger::warn("%s should only contain ServiceConfigurations, found object of type: %s instead",
					serviceConfigFile.toUtf8().constData(), object_type.get_name().to_string().c_str());
				continue;
			}

			// All good
			mServiceConfigs.emplace_back(std::move(config));
		}

		nap::Logger::info("Loaded config '%s'", toLocalURI(serviceConfigFile.toStdString()).c_str());
		mCurrentConfigFilename = serviceConfigFile;
	}


	bool ServiceConfig::saveServiceConfig()
	{
		if (mCurrentConfigFilename.isNull())
		{
			nap::Logger::fatal("Cannot save service config, no filename has been set.");
			return false;
		}
		return saveServiceConfigAs(mCurrentConfigFilename);
	}


	bool ServiceConfig::saveServiceConfigAs(const QString& fileName)
	{
		// Gather list of configurations to save to disk
		nap::rtti::ObjectList objects;
		for (const auto& config : mServiceConfigs)
		{
			config->mID = config->mID.empty() ? nap::math::generateUUID() : config->mID;
			objects.emplace_back(config.get());
		}

		// Serialize the configurations to json
		nap::rtti::JSONWriter writer;
		nap::utility::ErrorState error;
		if (!serializeObjects(objects, writer, error))
		{
			nap::Logger::fatal(error.toString().c_str());
			return false;
		}

		// Write to disk
		std::string json = writer.GetJSON();
		std::ofstream config_file;
		config_file.open(fileName.toStdString());
		if (!config_file.is_open())
		{
			nap::Logger::fatal("Unable to open file (stream) for writing");
			return false;
		}

		config_file << json << std::endl;
		config_file.close();
		nap::Logger::info("Written '%s'", toLocalURI(fileName.toStdString()).c_str());

		// Associate file with config on success
		mCurrentConfigFilename = fileName;
		return true;
	}


	bool ServiceConfig::setAsDefault()
	{
		// Ensure file is set
		if (getFilename().isNull())
		{
			nap::Logger::fatal("No filename specified");
			return false;
		}

		// Clone current project information
		assert(mCore.isInitialized());
		const auto* project_info = mCore.getProjectInfo();
		std::unique_ptr<nap::ProjectInfo> new_info = nap::rtti::cloneObject(*project_info, mCore.getResourceManager()->getFactory());

		// Get data directory and create relative path
		QDir data_dir(QString::fromStdString(project_info->getProjectDir()));
		QString new_path = data_dir.relativeFilePath(getFilename());
		new_info->mServiceConfigFilename = new_path.toStdString();

		nap::rtti::JSONWriter writer;
		nap::utility::ErrorState error;
		if (!nap::rtti::serializeObject(*new_info, writer, error))
		{
			nap::Logger::fatal(error.toString());
			return false;
		}

		// Open output file
		std::ofstream output(project_info->getFilename(), std::ios::binary | std::ios::out);
		if (!error.check(output.is_open() && output.good(), "Failed to open %s for writing", project_info->getFilename().c_str()))
		{
			nap::Logger::fatal(error.toString());
			return false;
		}

		// Write to disk
		std::string json = writer.GetJSON();
		output.write(json.data(), json.size());
		nap::Logger::info("Updated project configuration: %s", new_path.toUtf8().constData());
		return true;
	}


	void ServiceConfig::copyServiceConfig()
	{
		mCurrentConfigFilename.clear();
		if (mCore.getProjectInfo()->hasServiceConfigFile())
		{
			// Get absolute path
			std::string patched = nap::utility::joinPath({ mCore.getProjectInfo()->getProjectDir(), 
				mCore.getProjectInfo()->mServiceConfigFilename });

			// Ensure it exists
			mCurrentConfigFilename = QString::fromStdString(patched);
			QFileInfo info(mCurrentConfigFilename);
			if (!info.exists())
			{
				nap::Logger::warn("Unable to find config file: %s", patched.c_str());
				mCurrentConfigFilename.clear();
			}
			nap::Logger::info("Loading config '%s'", toLocalURI(patched).c_str());
		}

		// Copy service configurations
		assert(mCore.isInitialized());
		mServiceConfigs.clear();
		std::vector<const nap::ServiceConfiguration*> configs = mCore.getServiceConfigs();
		for (const auto& config : configs)
		{
			mServiceConfigs.emplace_back(nap::rtti::cloneObject(*config, mCore.getResourceManager()->getFactory()));
		}
	}

}