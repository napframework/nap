// Local Includes
#include "core.h"
#include "resourcemanager.h"
#include "logger.h"
#include "serviceobjectgraphitem.h"
#include "objectgraph.h"
#include "projectinfomanager.h"
#include "rtti/jsonreader.h"
#include "python.h"

// External Includes
#include <iostream>
#include <utility/fileutils.h>
#include <packaginginfo.h>

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
	}


	Core::Core(std::unique_ptr<CoreExtension> coreExtension)
	{
		// Initialize default and move interface
		mTimer.reset();
		mTicks.fill(0);
		mExtension = std::move(coreExtension);
	}


	Core::~Core()
	{
		// In order to ensure a correct order of destruction we want our entities, components, etc. to be deleted before other services are deleted.
		// Because entities and components are managed and owned by the resource manager we explicitly delete this first.
		// Erase it
		mResourceManager.reset();
	}


	bool Core::initializeEngine(utility::ErrorState& error, const std::string& forcedDataPath, bool runningInNonProjectContext)
	{
		// Ensure our current working directory is where the executable is.
		// Works around issues with the current working directory not being set as
		// expected when apps are launched directly from macOS Finder and probably other things too.
		nap::utility::changeDir(nap::utility::getExecutableDir());

		// Setup our Python environment
#ifdef NAP_ENABLE_PYTHON
		setupPythonEnvironment();
#endif

		// Load our module names from the project info
		ProjectInfo projectInfo;
		if (!runningInNonProjectContext)
		{
			if (!loadProjectInfoFromFile(*this, projectInfo, error))
				return false;
		}

		// Find our project data
		if (!determineAndSetWorkingDirectory(error, forcedDataPath))
			return false;

		// Create the resource manager
		mResourceManager = std::make_unique<ResourceManager>(*this);

		// Create the module manager
		mModuleManager = std::make_unique<ModuleManager>(*this);

		// Load modules
		if (!mModuleManager->loadModules(projectInfo.mModules, error))
			return false;

		// Create the various services based on their dependencies
		if (!createServices(error))
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


	void Core::calculateFramerate(uint32 tick)
	{
		mTicksum -= mTicks[mTickIdx];		// subtract value falling off
		mTicksum += tick;					// add new value
		mTicks[mTickIdx] = tick;			// save new value so it can be subtracted later */
		if (++mTickIdx == mTicks.size())    // inc buffer index
		{
			mTickIdx = 0;
		}
		mFramerate = 1000.0f / (static_cast<float>(mTicksum) / static_cast<float>(mTicks.size()));
	}


	void Core::start()
	{
		mTimer.reset();
	}


	double Core::update(std::function<void(double)>& updateFunction)
	{
		// Get current time in milliseconds
		uint32 new_tick_time = mTimer.getTicks();

		// Calculate amount of milliseconds since last time stamp
		uint32 delta_ticks = new_tick_time - mLastTimeStamp;

		// Store time stamp
		mLastTimeStamp = new_tick_time;

		// Update framerate
		calculateFramerate(delta_ticks);

		// Get delta time in seconds
		double delta_time = static_cast<double>(delta_ticks) / 1000.0;

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


	bool Core::createServices(utility::ErrorState& errorState)
	{
		using ConfigurationByTypeMap = std::unordered_map<rtti::TypeInfo, std::unique_ptr<ServiceConfiguration>>;
		ConfigurationByTypeMap configuration_by_type;

		// If there is a config file, read the service configurations from it.
		// Note that having a config file is optional, but if there *is* one, it should be valid
        rtti::DeserializeResult deserialize_result;
        if (hasServiceConfiguration())
        {
            if (loadServiceConfiguration(deserialize_result, errorState))
            {
                for (auto& object : deserialize_result.mReadObjects)
                {
                    if (!errorState.check(object->get_type().is_derived_from<ServiceConfiguration>(), "Config.json should only contain ServiceConfigurations"))
                        return false;

                    std::unique_ptr<ServiceConfiguration> config = rtti_cast<ServiceConfiguration>(object);
                    configuration_by_type[config->getServiceType()] = std::move(config);
                }
            }
            else {
                errorState.fail("Failed to load config.json");
                return false;
            }
        }

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
			std::unique_ptr<ServiceConfiguration> service_configuration(service_configuration_type.create<ServiceConfiguration>());
			rtti::TypeInfo serviceType = service_configuration->getServiceType();

			// Insert it in the map if it doesn't exist. Note that if it does exist, the unique_ptr will go out of scope and the default
			// object will be destroyed
			ConfigurationByTypeMap::iterator pos = configuration_by_type.find(serviceType);
			if (pos == configuration_by_type.end())
				configuration_by_type.insert(std::make_pair(serviceType, std::move(service_configuration)));
		}

		// First create and add all the services (unsorted)
		std::vector<Service*> services;
		for (auto& module : mModuleManager->mModules)
		{
			if (module.mService == rtti::TypeInfo::empty())
				continue;

			// Find the ServiceConfiguration that should be used to construct this service (if any)
			ServiceConfiguration* configuration = nullptr;
			ConfigurationByTypeMap::iterator pos = configuration_by_type.find(module.mService);
			if (pos != configuration_by_type.end())
				configuration = pos->second.release();

			// Create the service
			if (!addService(module.mService, configuration, services, errorState))
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
			const std::string packagedAppPythonPath = exeDir + "/python36.zip";
			_putenv_s("PYTHONPATH", packagedAppPythonPath.c_str());
		}
		else {
			// Set PYTHONPATH for thirdparty location beside NAP source
			const std::string napRoot = exeDir + "/../..";
			const std::string pythonHome = napRoot + "/../thirdparty/python/msvc/python-embed-amd64/python36.zip";
			_putenv_s("PYTHONPATH", pythonHome.c_str());
		}
#elif ANDROID
		// TODO ANDROID Need's to be implemented'
		Logger::warn("Python environment not setup for Android");
#else
		if (packagedBuild)
		{
			// Check for packaged app modules dir
			const std::string packagedAppPythonPath = exeDir + "/lib/python3.6";
			if (utility::dirExists(packagedAppPythonPath)) {
				setenv("PYTHONHOME", exeDir.c_str(), 1);
			}
			else {
				// Set PYTHONHOME to thirdparty location within packaged NAP release
				const std::string napRoot = exeDir + "/../../../../";
				const std::string pythonHome = napRoot + "/thirdparty/python/";
				setenv("PYTHONHOME", pythonHome.c_str(), 1);
			}
		}
		else {
			// set PYTHONHOME for thirdparty location beside NAP source
			const std::string napRoot = exeDir + "/../../";
			const std::string pythonHome = napRoot + "/../thirdparty/python/" + platformPrefix + "/install";
			setenv("PYTHONHOME", pythonHome.c_str(), 1);
		}
#endif
	}
}
