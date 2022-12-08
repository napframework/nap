/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "core.h"
#include "resourcemanager.h"
#include "logger.h"
#include "serviceobjectgraphitem.h"
#include "objectgraph.h"
#include "packaginginfo.h"
#include "python.h"

// External Includes
#include <iostream>
#include <utility/fileutils.h>
#include <rtti/jsonreader.h>
#include <rtti/jsonwriter.h>

// Temporarily bring in stdlib.h for PYTHONHOME environment variable setting
#if defined(__APPLE__) || defined(__unix__)
	#include <stdlib.h>
#endif

RTTI_BEGIN_CLASS(nap::Core)
	RTTI_FUNCTION("getService", (nap::Service* (nap::Core::*)(const std::string&))&nap::Core::getService)
	RTTI_FUNCTION("getResourceManager", &nap::Core::getResourceManager)
RTTI_END_CLASS


namespace nap
{
	/**
	@brief Constructor

	Creates a default entity as the root
	**/
	Core::Core()
	{
		// Initialize timer
		mTimer.reset();
		mTicks.fill(0);
		mResourceManager = std::make_unique<ResourceManager>(*this);
		mModuleManager = std::make_unique<ModuleManager>(*this);
	}


	Core::Core(std::unique_ptr<CoreExtension> coreExtension) : Core()
	{
		mExtension = std::move(coreExtension);
	}


	Core::~Core()
	{
		// After that remove all services
		mServices.clear();

		// Delete all service configurations
		mServiceConfigs.clear();

		// Delete all module references
		mModuleManager.reset(nullptr);

		// Delete project information
		mProjectInfo.reset(nullptr);

		// Delete extensions
		mExtension.reset(nullptr);
	}


	bool Core::initializeEngine(utility::ErrorState& error)
	{
		// Resolve project file path
		std::string project_file_path;
		if (!error.check(findProjectInfoFile(project_file_path),
			"Failed to find %s", PROJECT_INFO_FILENAME))
			return false;

		// Initialize engine
		return initializeEngine(project_file_path, ProjectInfo::EContext::Application, error);
	}


	bool Core::initializeEngine(const std::string& projectInfofile, ProjectInfo::EContext context, utility::ErrorState& error)
	{
		// Load project information
		assert(!mInitialized);
		if (!loadProjectInfo(projectInfofile, context, error))
			return false;

		// Ensure our current working directory is where the executable is.
		// Works around issues with the current working directory not being set as
		// expected when apps are launched directly from macOS Finder and probably other things too.
		nap::utility::changeDir(nap::utility::getExecutableDir());

		// Setup our Python environment
#ifdef NAP_ENABLE_PYTHON
		setupPythonEnvironment();
#endif
		// Apply any platform specific environment setup
		setupPlatformSpecificEnvironment();

		// Load modules
		if (!mModuleManager->loadModules(*mProjectInfo, error))
			return false;

		// If there is a config file, read the service configurations from it.
		// Note that having a config file is optional, but if there *is* one, it should be valid
		if(!loadServiceConfigurations(error))
			return false;
		
		// Always create services! 
		// Failure to create all the required services will result in certain objects unable to be created!
		// Creation is therefore required, in every context. Initialization happens later. 
		// After creation we're able to access all special object creation functions.
		// A default service configuration resource is created for all services that require one and
		// isn't already created during *loadServiceConfigurations()*
		if (!createServices(*mProjectInfo, error))
			return false;

		mInitialized = true;
		return true;
	}


	bool Core::initializePython(utility::ErrorState& error)
	{
#ifdef NAP_ENABLE_PYTHON
		// Here we register a callback that is called when the nap python module is imported.
		// We register a 'core' attribute so that we can write nap.core.<function>() in python
		// to access core functionality as a 'global'.
		nap::rtti::PythonModule::get("nap").registerImportCallback([this](pybind11::module& module)
		{
			module.attr("core") = this;
		});
#endif
		return true;
	}


	Core::ServicesHandle Core::initializeServices(utility::ErrorState& errorState)
	{
		// Attempt to initialize services
		Core::ServicesHandle services_handle(new Core::Services(*this, errorState));
		if (!services_handle->initialized())
			return nullptr;

		// Listen to potential resource file changes and 
		// forward those to all registered services
		mResourceManager->mPreResourcesLoadedSignal.connect(mPreResourcesLoadedSlot);
		mResourceManager->mPostResourcesLoadedSignal.connect(mPostResourcesLoadedSlot);

		// Return service handle
		return std::move(services_handle);
	}


	void Core::preResourcesLoaded()
	{
		for (auto& service : mServices)
			service->preResourcesLoaded();
	}


	void Core::postResourcesLoaded()
	{
		for (auto& service : mServices)
			service->postResourcesLoaded();
	}


	void Core::calculateFramerate(double deltaTime)
	{
		mTicksum -= mTicks[mTickIdx];			// subtract value falling off
		mTicksum += deltaTime;					// add new value
		mTicks[mTickIdx] = deltaTime;			// save new value so it can be subtracted later */
		if (++mTickIdx == mTicks.size())		// inc buffer index
			mTickIdx = 0;
		mFramerate = static_cast<double>(mTicks.size()) / mTicksum;
	}


	void Core::start()
	{
		mTimer.start();
		mTimeStamp = mTimer.getStartTime();
	}


	double Core::update(std::function<void(double)>& updateFunction)
	{
		// Calculate frame duration seconds
		SteadyTimeStamp current_time = SteadyClock::now();
		double delta_time = std::chrono::duration<double>(current_time - mTimeStamp).count();
		mTimeStamp = current_time;

		// Update framerate
		calculateFramerate(delta_time);

		// Perform update call before we check for file changes
		for (auto& service : mServices)
			service->preUpdate(delta_time);

		// Check for file changes
		mResourceManager->checkForFileChanges();

		// Update rest of the services
		for (auto& service : mServices)
			service->update(delta_time);

		// Call update function
		updateFunction(delta_time);

		// Update rest of the services
		for (auto& service : mServices)
			service->postUpdate(delta_time);

		return delta_time;
	}


	bool nap::Core::createServices(const nap::ProjectInfo& projectInfo, nap::utility::ErrorState& errorState)
	{
		// Gather all service configuration types
		std::vector<rtti::TypeInfo> service_configuration_types;
		rtti::getDerivedTypesRecursive(RTTI_OF(ServiceConfiguration), service_configuration_types);

		// For any ServiceConfigurations which weren't present in the config file, construct a default version of it,
		// so the service doesn't get a nullptr for its ServiceConfiguration if it depends on one
		for (const rtti::TypeInfo& service_configuration_type : service_configuration_types)
		{
			// Skip base class, not associated with any service.
			if (service_configuration_type == RTTI_OF(ServiceConfiguration))
				continue;

			assert(service_configuration_type.is_valid());
			assert(service_configuration_type.can_create_instance());

			// Construct the service configuration, store in unique ptr
			std::unique_ptr<ServiceConfiguration> service_config(service_configuration_type.create<ServiceConfiguration>());
			rtti::TypeInfo service_type = service_config->getServiceType();

			// Check if the service associated with the configuration isn't already part of the map, if so add as default
			// Config is automatically destructed otherwise.
			if (findServiceConfig(service_type) == nullptr)
				addServiceConfig(std::move(service_config));
		}

		// First create and add all the services (unsorted)
		std::vector<Service*> services;
		for (const auto& module : mModuleManager->mModules)
		{
			// No service associated with module
			if (module->getServiceType() == rtti::TypeInfo::empty())
				continue;

			// Find the ServiceConfiguration that should be used to construct this service (if any)
			ServiceConfiguration* configuration = findServiceConfig(module->getServiceType());

			// Create the service
			if (!addService(module->getServiceType(), configuration, services, errorState))
				return false;
		}

		// Create dependency graph
		ObjectGraph<ServiceObjectGraphItem> graph;

		// Build service dependency graph
		bool success = graph.build(services, [&](Service* service)
		{
			return ServiceObjectGraphItem::create(service, &services);
		}, errorState);

		// Make sure the graph was successfully build
		if (!errorState.check(success, "unable to build service dependency graph"))
			return false;

		// Add services in right order
		for (auto& node : graph.getSortedNodes())
		{
			// Add the service to core
			nap::Service* service = node->mItem.mObject;
			mServices.emplace_back(std::unique_ptr<nap::Service>(service));

			// This happens within this loop so services are able to query their dependencies while registering object creators
			service->registerObjectCreators(mResourceManager->getFactory());

			// Notify the service that is has been created and its core pointer is available
			// We put this call here so the service's dependencies are present and can already be queried if necessary
			service->created();
		}
		return true;
	}


	// Returns service that matches @type
	Service* Core::getService(const rtti::TypeInfo& type)
	{
		// Find service of type
		const auto& found_service = std::find_if(mServices.begin(), mServices.end(), [&type](const auto& service)
		{
			return rtti::isTypeMatch(service->get_type(), type, rtti::ETypeCheck::EXACT_MATCH);
		});

		// Check if found
		return found_service == mServices.end() ? nullptr : (*found_service).get();
	}


	nap::Service* Core::getService(const std::string& type)
	{
		rtti::TypeInfo stype = rtti::TypeInfo::get_by_name(type.c_str());
		return getService(stype);
	}


	const nap::Service* Core::getService(const rtti::TypeInfo& type) const
	{
		// Find service of type
		const auto& found_service = std::find_if(mServices.begin(), mServices.end(), [&type](const auto& service)
			{
				return rtti::isTypeMatch(service->get_type(), type, rtti::ETypeCheck::EXACT_MATCH);
			});

		// Check if found
		return found_service == mServices.end() ? nullptr : (*found_service).get();
	}


	// Add a new service
	bool Core::addService(const rtti::TypeInfo& type, ServiceConfiguration* configuration, std::vector<Service*>& outServices, utility::ErrorState& errorState)
	{
		assert(type.is_valid());
		assert(type.can_create_instance());
		assert(type.is_derived_from<Service>());

		// Check if service doesn't already exist
		const auto& found_service = std::find_if(outServices.begin(), outServices.end(), [&type](const auto& service)
		{
			return service->get_type() == type.get_raw_type();
		});

		bool new_service = found_service == outServices.end();
		if (!errorState.check(new_service, "can't add service of type: %s, service already exists", type.get_name().data()))
			return false;

		// Add service
		Service* service = type.create<Service>({ configuration });
		service->mCore = this;
		outServices.emplace_back(service);

		return true;
	}


	// return number of elapsed ticks
	uint32 Core::getTicks() const
	{
		return mTimer.getTicks();
	}


	// Return elapsed time
	double Core::getElapsedTime() const
	{
		return mTimer.getElapsedTime();
	}


	void Core::setupPythonEnvironment()
	{
#ifdef _WIN32
		const std::string platformPrefix = "msvc";
#elif defined(__APPLE__)
		const std::string platformPrefix = "macos";
#else // __unix__
		const std::string platformPrefix = "linux";
#endif

#ifdef NAP_PACKAGED_BUILD
		const bool packagedBuild = true;
#else
		const bool packagedBuild = false;
#endif

		const std::string exeDir = utility::getExecutableDir();

#if _WIN32
		if (packagedBuild)
		{
			// TODO Explore locating Python instead in third party to reduce duplication on disk
			// We have our Python modules zip alongside our executable for running against NAP source or packaged apps
			const std::string packagedAppPythonPath = utility::joinPath({exeDir, "python36.zip"});
			_putenv_s("PYTHONPATH", packagedAppPythonPath.c_str());
		}
		else {
			// Set PYTHONPATH for thirdparty location beside NAP source
			const std::string pythonHome = utility::joinPath({mProjectInfo->getNAPRootDir(), "thirdparty", "python", "msvc", "x86_64", "python36.zip"});
			_putenv_s("PYTHONPATH", pythonHome.c_str());
		}
#else
        // Check for packaged app modules dir
        const std::string packagedAppPythonPath = utility::joinPath({mProjectInfo->getProjectDir(), "lib", "python3.6"});
        if (utility::dirExists(packagedAppPythonPath)) {
            setenv("PYTHONHOME", mProjectInfo->getProjectDir().c_str(), 1);
        }
        else {
            // set PYTHONHOME for thirdparty location within NAP source
            const std::string pythonHome = utility::joinPath({mProjectInfo->getNAPRootDir(), "thirdparty", "python", platformPrefix, sBuildArch});
            setenv("PYTHONHOME", pythonHome.c_str(), 1);
        }
#endif
	}


	nap::ServiceConfiguration* Core::findServiceConfig(rtti::TypeInfo type) const
	{
		auto it = mServiceConfigs.find(type);
		return it == mServiceConfigs.end() ? nullptr : it->second.get();
	}


	bool Core::addServiceConfig(std::unique_ptr<nap::ServiceConfiguration> serviceConfig)
	{
		rtti::TypeInfo service_type = serviceConfig->getServiceType();
		auto return_v = mServiceConfigs.emplace(std::make_pair(service_type, std::move(serviceConfig)));
		return return_v.second;
	}


	bool nap::Core::loadProjectInfo(std::string projectFilename, ProjectInfo::EContext context, nap::utility::ErrorState& err)
	{
		// Load ProjectInfo from json
		mProjectInfo = nap::rtti::getObjectFromJSONFile<nap::ProjectInfo>(
			projectFilename.c_str(),
			nap::rtti::EPropertyValidationMode::DisallowMissingProperties,
			getResourceManager()->getFactory(),
			err);

		// Ensure project info is loaded correctly.
		if (!err.check(mProjectInfo != nullptr,
			"Failed to load project info %s", projectFilename.c_str()))
			return false;

		// Set if paths are resolved for editor or application
		mProjectInfo->mContext = context;

		// Store originating filename so we can reference it later and load path mapping
		mProjectInfo->mFilename = projectFilename;
		if(!loadPathMapping(*mProjectInfo, err))
			return false;

		// Ensure templates/variables are replaced with their intended values
		mProjectInfo->patchPath(mProjectInfo->mServiceConfigFilename);
		if (!err.check(mProjectInfo->mPathMapping != nullptr,
					   "Failed to load path mapping %s: %s",
					   mProjectInfo->mPathMappingFile.c_str(), err.toString().c_str()))
			return false;

		// Notify project info is loaded
		nap::Logger::info("Loading project '%s' ver. %s (%s)",
						  mProjectInfo->mTitle.c_str(),
						  mProjectInfo->mVersion.c_str(), mProjectInfo->getProjectDir().c_str());
		return true;
	}


	bool nap::Core::loadServiceConfigurations(nap::utility::ErrorState& err)
	{
		// No service configuration specified
		if (!mProjectInfo->hasServiceConfigFile())
			return true;

		// Extract all user defined service configurations
		rtti::DeserializeResult deserialize_result;
		utility::ErrorState deserialize_error;
		if (loadServiceConfiguration(mProjectInfo->mServiceConfigFilename, deserialize_result, deserialize_error))
		{
			for (std::unique_ptr<rtti::Object>& object : deserialize_result.mReadObjects)
			{
				// Check if it's indeed a service configuration object
				rtti::TypeInfo object_type = object->get_type();
				std::unique_ptr<ServiceConfiguration> config = rtti_cast<ServiceConfiguration>(object);

				if (!err.check(config != nullptr,
					"%s should only contain ServiceConfigurations, found object of type: %s instead",
						mProjectInfo->mServiceConfigFilename.c_str(), object_type.get_name().to_string().c_str()))
					return false;

				// Get type before moving and store pointer for
                nap::ServiceConfiguration* config_ptr = config.get();
				bool added = addServiceConfig(std::move(config));

				// Duplicates are not allowed
				if(!err.check(added, "Duplicate service configuration found with id: %s, type: %s",
							   config_ptr->mID.c_str(), config_ptr->getServiceType().get_name().to_string().c_str()))
					return false;
			}
		}
		else
		{
			// File doesn't exist or can't be deserialized
			nap::Logger::warn(deserialize_error.toString().c_str());
		}
		return true;
	}


	bool Core::loadPathMapping(nap::ProjectInfo& projectInfo, nap::utility::ErrorState& err)
	{
		// Load path mapping (relative to the app.json file)
		auto pathMappingFilename = utility::joinPath({projectInfo.getProjectDir(), projectInfo.mPathMappingFile});
		projectInfo.mPathMapping = nap::rtti::getObjectFromJSONFile<nap::PathMapping>(
			pathMappingFilename,
			nap::rtti::EPropertyValidationMode::DisallowMissingProperties,
			getResourceManager()->getFactory(),
			err);

		if (err.hasErrors())
		{
			nap::Logger::error("Failed to load path mapping %s: %s", pathMappingFilename.c_str(), err.toString().c_str());
			return false;
		}

		// Template/variable replacement
		projectInfo.patchPaths(projectInfo.mPathMapping->mModulePaths);
		return true;
	}


    bool Core::writeConfigFile(utility::ErrorState& errorState)
    {
	    // Write all available service configurations to a vector
        std::vector<rtti::Object*> objects;
        for (auto& service : mServices)
        {
            auto configuration = service->getConfiguration<rtti::Object>();
            if (configuration != nullptr)
            {
                // The serializer needs all objects to have a mID set
                if (configuration->mID.empty())
                    configuration->mID = configuration->get_type().get_name().to_string();
                objects.emplace_back(configuration);
            }
        }

        // Serialize the configurations to json
        rtti::JSONWriter writer;
        if (!serializeObjects(objects, writer, errorState))
            return false;
        std::string json = writer.GetJSON();

        // Save the config file besides the binary, the first location that NAP searches
		assert(getProjectInfo() != nullptr);
        const std::string configFilePath = getProjectInfo()->getProjectDir() + "/" + DEFAULT_SERVICE_CONFIG_FILENAME;

        std::ofstream configFile;
        configFile.open(configFilePath);
        configFile << json << std::endl;
        configFile.close();
		nap::Logger::info("Wrote configuration to: %s", configFilePath.c_str());

        return true;
    }


	const nap::ProjectInfo* Core::getProjectInfo() const
	{
		return mProjectInfo.get();
	}


	bool Core::isInitialized() const
	{
		return mInitialized;
	}


	std::vector<const ServiceConfiguration*> Core::getServiceConfigs() const
	{
		std::vector<const ServiceConfiguration*> configs;
		configs.reserve(mServiceConfigs.size());
		for (const auto& config : mServiceConfigs)
		{
			configs.emplace_back(config.second.get());
		}
		return configs;
	}


	nap::SteadyTimeStamp Core::getStartTime() const
	{
		return mTimer.getStartTime();
	}


	Core::Services::~Services()
	{
		// Call pre-shutdown on services to give them a chance to reset any state they need
		// before resources are destroyed.
		for (auto it = mCore.mServices.rbegin(); it != mCore.mServices.rend(); it++)
		{
			Service& service = **it;
			service.preShutdown();
		}

		// Destroy the resource manager, which will destroy all loaded resources
		mCore.mResourceManager.reset();

		// Shutdown services after all resources have been destroyed.
		// This order ensures that resources can still make use of services during destruction
		for (auto it = mCore.mServices.rbegin(); it != mCore.mServices.rend(); it++)
		{
			Service& service = **it;
			nap::Logger::debug("shutting down service: %s", service.getTypeName().c_str());
			service.shutdown();
		}
	}


	Core::Services::Services(Core& core, utility::ErrorState& error) : mCore(core)
	{
		// Initialize all services one by one
		std::vector<Service*> objects;
		std::string si;
		for (const auto& service : mCore.mServices)
		{
			si = utility::stringFormat("Initializing service: %s", service->getTypeName().c_str());
			std::cout << std::string(si.size(), '-') << std::endl;
			nap::Logger::info(si);

			if (!service->init(error))
			{
				error.fail("Failed to initialize services");
				return;
			}
		}
		std::cout << std::string(si.size(), '-') << std::endl;
		mInitialized = true;
	}
}
