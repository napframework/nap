/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
#include <mathutils.h>
#include <appcontext.h>

namespace napkin
{
	ServiceConfig::ServiceConfig(nap::Core& core, nap::ProjectInfo& projectInfo) : QObject(),
		mCore(core), mProjectInfo(projectInfo)
	{
		// Extract filename of project default
		QString file_name;
		if (mProjectInfo.hasServiceConfigFile())
		{
			// Get absolute path
			std::string patched = nap::utility::joinPath({ mCore.getProjectInfo()->getProjectDir(),
				mCore.getProjectInfo()->mServiceConfigFilename });
			nap::Logger::info("Loading config '%s'", toLocalURI(patched).c_str());

			// Ensure it exists
			file_name = QString::fromStdString(patched);
			QFileInfo info(file_name);
			if (!info.exists())
			{
				nap::Logger::warn("Unable to find config file: %s", patched.c_str());
				file_name.clear();
			}
		}

		// Create copies to edit
		nap::rtti::OwnedObjectList configs = copyServiceConfig();

		// Create document
		mDocument = std::make_unique<Document>(mCore, file_name, std::move(configs));
	}


	const QString& ServiceConfig::getFilename() const
	{
		return mDocument->getFilename();
	}


	void ServiceConfig::create()
	{
		// Create new set of default service configurations
		assert(mCore.isInitialized());
		std::vector<const nap::ServiceConfiguration*> current_configs = mCore.getServiceConfigs();
		nap::rtti::OwnedObjectList configs;
		configs.reserve(current_configs.size());

		// Use the rtti type to create a new default instance and set ID
		for (const auto& config : current_configs)
		{
			nap::rtti::Object* obj = config->get_type().create<nap::rtti::Object>();
			std::string type_name = nap::utility::stripNamespace(obj->get_type().get_name().to_string());
			obj->mID = nap::utility::stringFormat("%s_%s", type_name.c_str(), nap::math::generateUUID().c_str());
			configs.emplace_back(obj);
		}

		// Create copies to edit
		AppContext::get().serviceConfigurationClosing(mDocument->getFilename());
		mDocument = std::make_unique<Document>(mCore, QString(), std::move(configs));
		nap::Logger::info("Created new default configuration");
		AppContext::get().serviceConfigurationChanged();
	}


	void ServiceConfig::load(QString serviceConfigFile)
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
		nap::rtti::OwnedObjectList configs;
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
			configs.emplace_back(std::move(config));
		}

		AppContext::get().serviceConfigurationClosing(mDocument->getFilename());
		mDocument = std::make_unique<Document>(mCore, serviceConfigFile, std::move(configs));
		nap::Logger::info("Loaded config '%s'", toLocalURI(serviceConfigFile.toStdString()).c_str());
		AppContext::get().serviceConfigurationChanged();
	}


	bool ServiceConfig::save()
	{
		if (mDocument->getFilename().isNull())
		{
			nap::Logger::fatal("Cannot save service config, no filename has been set.");
			return false;
		}
		return saveAs(mDocument->getFilename());
	}


	bool ServiceConfig::saveAs(const QString& fileName)
	{
		// Gather list of configurations to save to disk
		nap::rtti::ObjectList objects;
		for (const auto& config : mDocument->getObjects())
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
		mDocument->setFilename(fileName);
		return true;
	}


	bool ServiceConfig::isProjectDefault() const
	{
		// Get relative path
		QDir proj_dir(QString::fromStdString(mProjectInfo.getProjectDir()));
		std::string config_file = proj_dir.relativeFilePath(getFilename()).toStdString();

		// Compare to cached service config file
		return mProjectInfo.mServiceConfigFilename == config_file;
	}


	bool ServiceConfig::makeProjectDefault()
	{
		// Ensure file is set
		if (getFilename().isEmpty())
		{
			nap::Logger::fatal("No filename specified");
			return false;
		}

		// Get data directory and create relative path
		QDir proj_dir(QString::fromStdString(mProjectInfo.getProjectDir()));
		QString new_path = proj_dir.relativeFilePath(getFilename());
		mProjectInfo.mServiceConfigFilename = new_path.toStdString();

		nap::rtti::JSONWriter writer;
		nap::utility::ErrorState error;
		if (!nap::rtti::serializeObject(mProjectInfo, writer, error))
		{
			nap::Logger::fatal(error.toString());
			return false;
		}

		// Open output file
		std::ofstream output(mProjectInfo.getFilename(), std::ios::binary | std::ios::out);
		if (!error.check(output.is_open() && output.good(), "Failed to open %s for writing", mProjectInfo.getFilename().c_str()))
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


	std::vector<nap::ServiceConfiguration*> ServiceConfig::getList() const
	{
		std::vector<nap::ServiceConfiguration*> configs;
		configs.reserve(mDocument->getObjects().size());
		for (const auto& obj : mDocument->getObjects())
		{
			if (obj->get_type().is_derived_from(RTTI_OF(nap::ServiceConfiguration)))
			{
				configs.emplace_back(static_cast<nap::ServiceConfiguration*>(obj.get()));
			}
		}
		return configs;
	}


	nap::rtti::OwnedObjectList ServiceConfig::copyServiceConfig()
	{
		// Copy service configurations
		assert(mCore.isInitialized());
		nap::rtti::OwnedObjectList config_copies;
		std::vector<const nap::ServiceConfiguration*> configs = mCore.getServiceConfigs();
		for (const auto& config : configs)
		{
			std::unique_ptr<nap::rtti::Object> obj = nap::rtti::cloneObject(*config, mCore.getResourceManager()->getFactory());
			if (obj->mID.empty())
			{
				std::string type_name = nap::utility::stripNamespace(obj->get_type().get_name().to_string());
				obj->mID = nap::utility::stringFormat("%s_%s", type_name.c_str(), nap::math::generateUUID().c_str());
			}
			config_copies.emplace_back(std::move(obj));
		}
		return config_copies;
	}
}
