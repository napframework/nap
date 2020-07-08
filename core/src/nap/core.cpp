// Local Includes
#include "core.h"
#include "resourcemanager.h"
#include "logger.h"
#include "serviceobjectgraphitem.h"
#include "objectgraph.h"
#include "projectinfomanager.h"
#include "rtti/jsonreader.h"
#include <rtti/jsonwriter.h>
#include "python.h"
#include "packaginginfo.h"

// External Includes
#include <iostream>
#include <utility/fileutils.h>

// Temporarily bring in stdlib.h for PYTHONHOME environment variable setting
#if defined(__APPLE__) || defined(__unix__)
	#include <stdlib.h>
#endif

using namespace std;

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


	Core::Core(std::unique_ptr<CoreExtension> coreExtension)
	{
		// Initialize default and move interface
		mTimer.reset();
		mTicks.fill(0);
		mResourceManager = std::make_unique<ResourceManager>(*this);
		mModuleManager = std::make_unique<ModuleManager>(*this);

		mExtension = std::move(coreExtension);
	}


	Core::~Core()
	{
		// In order to ensure a correct order of destruction we want our entities, components, etc. to be deleted before other services are deleted.
		// Because entities and components are managed and owned by the resource manager we explicitly delete this first.
		// Erase it
		mResourceManager.reset();
	}


	bool Core::initializeEngine(utility::ErrorState& error)
	{

		if (!loadProjectInfo(error))
			return false;

		return doInitializeEngine(error);
	}

	bool nap::Core::initializeEngine(nap::utility::ErrorState& error, std::unique_ptr<nap::ProjectInfo> projectInfo)
	{
		mProjectInfo = std::move(projectInfo);

		return doInitializeEngine(error);
	}

	bool nap::Core::doInitializeEngine(utility::ErrorState& error)
	{
		// Ensure our current working directory is where the executable is.
		// Works around issues with the current working directory not being set as
		// expected when apps are launched directly from macOS Finder and probably other things too.
		nap::utility::changeDir(nap::utility::getExecutableDir());

#ifdef NAP_ENABLE_PYTHON
		setupPythonEnvironment();
#endif

		// Change directory to data folder
		std::string dataDir = mProjectInfo->dataDirectory();
		if (!error.check(utility::fileExists(dataDir), "Data path does not exist: %s", dataDir.c_str()))
			return false;
		utility::changeDir(dataDir);

		// Load modules
		if (!mModuleManager->loadModules(*mProjectInfo, error))
			return false;

		// Create the various services based on their dependencies
		if (!initializeServices(*mProjectInfo, error))
			return false;

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


	bool Core::initializeServices(utility::ErrorState& errorState)
	{
		// Initialize all services one by one
		std::vector<Service*> objects;
		for (const auto& service : mServices)
		{
			nap::Logger::info("initializing service: %s", service->getTypeName().c_str());
			if (!service->init(errorState))
				return false;
		}

		// Listen to potential resource file changes and 
		// forward those to all registered services
		mResourceManager->mFileLoadedSignal.connect(mFileLoadedSlot);

		return true;
	}


	void Core::resourceFileChanged(const std::string& file)
	{
		for (auto& service : mServices)
		{
			service->resourcesLoaded();
		}
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
		mTimer.reset();
	}


	double Core::update(std::function<void(double)>& updateFunction)
	{
		// Get current time in milliseconds
		double new_elapsed_time = mTimer.getElapsedTime();

		// Calculate amount of milliseconds since last time stamp
		double delta_time = new_elapsed_time - mLastTimeStamp;

		// Store time stamp
		mLastTimeStamp = new_elapsed_time;

		// Update framerate
		calculateFramerate(delta_time);

		// Perform update call before we check for file changes
		for (auto& service : mServices)
		{
			service->preUpdate(delta_time);
		}

		// Check for file changes
		mResourceManager->checkForFileChanges();

		// Update rest of the services
		for (auto& service : mServices)
		{
			service->update(delta_time);
		}

		// Call update function
		updateFunction(delta_time);

		// Update rest of the services
		for (auto& service : mServices)
		{
			service->postUpdate(delta_time);
		}

		return delta_time;
	}


	void Core::shutdownServices()
	{
		mResourceManager.reset();

		for (auto it = mServices.rbegin(); it != mServices.rend(); it++)
		{
			Service& service = **it;
			nap::Logger::debug("shutting down service: %s", service.getTypeName().c_str());
			service.shutdown();
		}
	}

	bool nap::Core::initializeServices(const nap::ProjectInfo& projectInfo, nap::utility::ErrorState& errorState)
	{
		// Convert service configurations to map
		ServiceConfigMap configuration_by_type;
		for (auto& config : projectInfo.mServiceConfigurations)
			configuration_by_type[config->getServiceType()] = config;

		// Gather all service configuration types
		std::vector<rtti::TypeInfo> service_configuration_types;
		rtti::getDerivedTypesRecursive(RTTI_OF(ServiceConfiguration), service_configuration_types);

		// For any ServiceConfigurations which weren't present in the config file, construct a default version of it,
		// so the service doesn't get a nullptr for its ServiceConfiguration if it depends on one
		for (const rtti::TypeInfo& service_configuration_type : service_configuration_types)
		{
			if (service_configuration_type == RTTI_OF(ServiceConfiguration))
				continue;

			assert(service_configuration_type.is_valid());
			assert(service_configuration_type.can_create_instance());

			// Construct the service configuration
			// TODO: Free this allocation
			auto service_configuration(service_configuration_type.create<ServiceConfiguration>());
			rtti::TypeInfo serviceType = service_configuration->getServiceType();

			auto pos = configuration_by_type.find(serviceType);
			if (pos == configuration_by_type.end())
				configuration_by_type.insert(std::make_pair(serviceType, service_configuration));
		}

		// First create and add all the services (unsorted)
		std::vector<Service*> services;
		for (const auto& module : mModuleManager->mModules)
		{
			if (module->mService == rtti::TypeInfo::empty())
				continue;

			// Find the ServiceConfiguration that should be used to construct this service (if any)
			ServiceConfiguration* configuration = nullptr;
			auto pos = configuration_by_type.find(module->mService);
			if (pos != configuration_by_type.end())
				configuration = pos->second;

			// Create the service
			if (!addService(module->mService, configuration, services, errorState))
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


	// Returns start time of core module as point in time
	HighResTimeStamp Core::getStartTime() const
	{
		return mTimer.getStartTime();
	}


	void Core::setupPythonEnvironment()
	{
#ifdef _WIN32
		const std::string platformPrefix = "msvc";
#elif defined(__APPLE__)
		const std::string platformPrefix = "osx";
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
			// We have our Python modules zip alongside our executable for running against NAP source or packaged apps
			const std::string packagedAppPythonPath = utility::joinPath({exeDir, "python36.zip"});
			_putenv_s("PYTHONPATH", packagedAppPythonPath.c_str());
		}
		else {
			// Set PYTHONPATH for thirdparty location beside NAP source
			const std::string pythonHome = utility::joinPath({mProjectInfo->getNAPRootDir(), "..", "thirdparty", "python", "msvc", "python-embed-amd64", "python36.zip"});
			_putenv_s("PYTHONPATH", pythonHome.c_str());
		}
#elif ANDROID
		// TODO ANDROID Need's to be implemented'
		Logger::warn("Python environment not setup for Android");
#else
		if (packagedBuild)
		{
			// Check for packaged app modules dir
			const std::string packagedAppPythonPath = utility::joinPath({mProjectInfo->getProjectDir(), "lib", "python3.6"});
			if (utility::dirExists(packagedAppPythonPath)) {
				setenv("PYTHONHOME", mProjectInfo->getProjectDir().c_str(), 1);
			}
			else {
				// Set PYTHONHOME to thirdparty location within packaged NAP release
				const std::string pythonHome = utility::joinPath({mProjectInfo->getNAPRootDir(), "thirdparty", "python"});
				setenv("PYTHONHOME", pythonHome.c_str(), 1);
			}
		}
		else {
			// set PYTHONHOME for thirdparty location beside NAP source
			const std::string pythonHome = utility::joinPath({mProjectInfo->getNAPRootDir(), "..", "thirdparty", "python", platformPrefix, "install"});
			setenv("PYTHONHOME", pythonHome.c_str(), 1);
		}
#endif
	}

	bool nap::Core::loadProjectInfo(nap::utility::ErrorState& err)
	{
		// Load project info
		std::string projectFilename;
		if (!err.check(findProjectFilePath(PROJECT_INFO_FILENAME, projectFilename),
					   "Failed to find %s", PROJECT_INFO_FILENAME))
			return false;

		// Load ProjectInfo from json
		mProjectInfo = nap::rtti::readJSONFileObjectT<nap::ProjectInfo>(
			projectFilename.c_str(),
			nap::rtti::EPropertyValidationMode::DisallowMissingProperties,
			nap::rtti::EPointerPropertyMode::OnlyRawPointers,
			getResourceManager()->getFactory(),
			err);

		if (!err.check(mProjectInfo != nullptr,
					  "Failed to load project info %s",
					  projectFilename.c_str()))
			return false;

		// Store originating filename so we can reference it later
		mProjectInfo->mFilename = projectFilename;

		// Load path mapping
		loadPathMapping(*mProjectInfo, err);

		if (!err.check(mProjectInfo->mPathMapping != nullptr,
					   "Failed to load path mapping %s: %s",
					   mProjectInfo->mPathMappingFile.c_str(), err.toString().c_str()))
			return false;

		nap::Logger::info("Loading project '%s' ver. %s (%s)",
						  mProjectInfo->mTitle.c_str(),
						  mProjectInfo->mVersion.c_str(), mProjectInfo->getProjectDir().c_str());

		return true;
	}

	bool Core::loadPathMapping(nap::ProjectInfo& projectInfo, nap::utility::ErrorState& err)
	{
		// Load path mapping (relative to the project.json file)
		auto pathMappingFilename = utility::joinPath({projectInfo.getProjectDir(), projectInfo.mPathMappingFile});
		auto pathMapping = nap::rtti::readJSONFileObjectT<nap::PathMapping>(
			pathMappingFilename,
			nap::rtti::EPropertyValidationMode::DisallowMissingProperties,
			nap::rtti::EPointerPropertyMode::OnlyRawPointers,
			getResourceManager()->getFactory(),
			err);

		if (err.hasErrors())
		{
			nap::Logger::error("Failed to load path mapping %s: %s", pathMappingFilename.c_str(), err.toString().c_str());
			return false;
		}

		// Store loaded path mapping first
		projectInfo.mPathMapping = std::move(pathMapping);

		std::unordered_map<std::string, std::string> reps = {
			{"ROOT", projectInfo.getNAPRootDir()},
			{"BUILD_CONFIG", sBuildConf},
			{"BUILD_TYPE", sBuildType},
			{"PROJECT_DIR", projectInfo.getProjectDir()},
		};

		// As EXE_DIR is the project execurable directory the editor won't be aware of this and shouldn't
		// try and populate it
		if (!projectInfo.isEditorMode())
			reps["EXE_DIR"] = utility::getExecutableDir();

		// Do string/template replacement
		projectInfo.mPathMapping->mModulePaths = utility::namedFormat(projectInfo.mPathMapping->mModulePaths, reps);

		return true;
	}

	nap::ProjectInfo* nap::Core::getProjectInfo()
	{
		return mProjectInfo.get();
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
        const std::string exeDir = utility::getExecutableDir();
        const std::string configFilePath = utility::getExecutableDir() + "/" + SERVICE_CONFIG_FILENAME;

        std::ofstream configFile;
        configFile.open(configFilePath);
        configFile << json << std::endl;
        configFile.close();

        return true;
    }

}
