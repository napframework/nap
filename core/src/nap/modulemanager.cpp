// Local Includes
#include "modulemanager.h"
#include "logger.h"
#include "service.h"
#include "module.h"
#include <nap/core.h>

// External Includes
#include <utility/fileutils.h>
#include <packaginginfo.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <fstream>

namespace nap
{
	ModuleManager::ModuleManager(nap::Core& core) :
		mCore(core)
	{
		initModules();
	}

	ModuleManager::~ModuleManager()
	{
		/*  Commented out for now because unloading modules can cause crashes in RTTR; during shutdown it will try
			to access RTTI types registered by potentially unloaded modules and crash because the pointers are no longer valid.
			
			This should probably be fixed in RTTR itself, but I'm not sure how yet. For now I've disabled the unloading of modules, 
			since this only happens during shutdown and modules will be unloaded then anyway.
		
 		for (Module& module : mModules)
 			UnloadModule(module.mHandle);
		*/
	}
	

	bool ModuleManager::folderNameContainsBuildConfiguration(const std::string& folderName)
	{
#ifdef NDEBUG
		const std::string runningBuildType = "Release";
#else
		const std::string runningBuildType = "Debug";
#endif
	
		if (runningBuildType != folderName)
			return false;
		
		return true;
	}
	

	void ModuleManager::buildPackagedNapNonProjectModulePaths(std::vector<std::string>& searchDirectories)
	{
		// Cater for Napkin running against packaged NAP.  Module names won't be specified.  Let's load everything we can find.
		
#ifdef NDEBUG
		const std::string buildType = "Release";
#else
		const std::string buildType = "Debug";
#endif
		
		std::string exeDir = utility::getExecutableDir();
		std::string napRoot = exeDir + "/../../../../";
		
		// NAP modules
		std::string searchDir = napRoot + "modules";
		std::vector<std::string> filesInDirectory;
		utility::listDir(searchDir.c_str(), filesInDirectory, false);
		for (const std::string& module: filesInDirectory)
		{
			std::string dirName = napRoot + "modules/" + module + "/lib/" + buildType;
			if (!utility::dirExists(dirName))
				continue;
			searchDirectories.push_back(dirName);
			//Logger::info("Adding module search path %s for napkin", dirName.c_str());
		}
		
		// User modules
		searchDir = napRoot + "user_modules";
		filesInDirectory.clear();
		utility::listDir(searchDir.c_str(), filesInDirectory, false);
		for (const std::string& module: filesInDirectory)
		{
			std::string dirName = napRoot + "user_modules/" + module + "/lib/" + buildType;
			if (!utility::dirExists(dirName))
				continue;
			searchDirectories.push_back(dirName);
			//Logger::info("Adding user module search path %s for napkin", dirName.c_str());
		}

		// Project module
		searchDirectories.push_back(exeDir + "/../../module/lib/" + buildType);
	}

	
	void ModuleManager::buildPackagedNapProjectModulePaths(const std::vector<std::string>& moduleNames, 
		                                                   std::vector<std::string>& searchDirectories)
	{
#ifdef NDEBUG
		const std::string buildType = "Release";
#else
		const std::string buildType = "Debug";
#endif
		
		std::string exeDir = utility::getExecutableDir();
		std::string napRoot = exeDir + "/../../../../";
		// Normal project running against packaged NAP
		for (const std::string& module : moduleNames)
		{
			// NAP modules
			searchDirectories.push_back(napRoot + "modules/" + module + "/lib/" + buildType);
			// User modules
			searchDirectories.push_back(napRoot + "user_modules/" + module + "/lib/" + buildType);
		}
		
		// Project module
		searchDirectories.push_back(exeDir + "/../../module/lib/" + buildType);
	}
	
	
	bool ModuleManager::loadModuleDependenciesFromJsonFile(const std::string& jsonFile, 
		                                               std::vector<std::string>& dependencies, 
		                                               utility::ErrorState& errorState)
	{
		// Ensure the JSON file exists
		if (!utility::fileExists(jsonFile)) 
		{
			errorState.fail("Couldn't load from " + jsonFile);
			return false;
		}
		
		// Open the file
		std::ifstream in(jsonFile, std::ios::in | std::ios::binary);
		if (!errorState.check(in.good(), "Unable to open file %s", jsonFile.c_str()))
			return false;
		
		// Create buffer of appropriate size
		in.seekg(0, std::ios::end);
		size_t len = in.tellg();
		std::string buffer;
		buffer.resize(len);
		
		// Read all data
		in.seekg(0, std::ios::beg);
		in.read(&buffer[0], len);
		in.close();
		
		return deserializeModuleDependenciesFromJson(buffer, dependencies, errorState);
	}

	bool ModuleManager::deserializeModuleDependenciesFromJson(const std::string& json, 
	                                                          std::vector<std::string>& dependencies, 
	                                                          utility::ErrorState& errorState)
	{
		// Try to parse the JSON file
		rapidjson::Document document;
		rapidjson::ParseResult parse_result = document.Parse(json.c_str());
		if (!parse_result)
		{
			errorState.fail("Error parsing json: %s", 
				            rapidjson::GetParseError_En(parse_result.Code()));
			return false;
		}
		
		// Basic validation
		rapidjson::Value::ConstMemberIterator dependenciesElement = document.FindMember("dependencies");
		if (!errorState.check(dependenciesElement != document.MemberEnd(), "Unable to find required 'dependencies' module info field"))
			return false;
		if (!errorState.check(dependenciesElement->value.IsArray(), "'dependencies' module info field must be an array"))
			return false;
		
		// Populate the output list
		for (std::size_t index = 0; index < dependenciesElement->value.Size(); ++index)
		{
			const rapidjson::Value& json_element = dependenciesElement->value[index];
			if (!errorState.check(json_element.IsString(), "Entries in 'dependencies' array in module info field must be a strings"))
				return false;
			
			dependencies.push_back(json_element.GetString());
		}
		
		return true;		
	}

}
