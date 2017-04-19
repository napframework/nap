#include <nap/core.h>
#include <nap/fileutils.h>

#include <thread>
#include <fstream>
#include <rapidjson/document.h>

#ifndef APP_NAME
#define APP_NAME "unknown"
#endif

int main(int argc, char** argv)
{
	// Parse command line
	printf("NAP Skeleton\n");

	bool keepAlive = true;

	// Setup core
	nap::Core core;
    
    std::string modulesDirectory = ".";

    // Try to read the config file
    std::string configFileName = nap::getAbsolutePath("config.json");
    if (nap::fileExists(configFileName))
    {
        std::ifstream stream(configFileName);
        std::string configStr((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        rapidjson::Document document;
        document.Parse(configStr.c_str());
        
        if (document.HasParseError())
        {
            nap::Logger::fatal("Failed to parse config file");
            return -1;
        }
        
        // Try to read modules directory
        if (document.HasMember("modulesDirectory"))
        {
            rapidjson::Value& modulesDirectoryJson = document["modulesDirectory"];
            if (!modulesDirectoryJson.IsString())
                nap::Logger::warn("Failed to read modules directory from config file: not a string");
            else {
                modulesDirectory = nap::getAbsolutePath(modulesDirectoryJson.GetString());
                if (!nap::dirExists(modulesDirectory))
                {
                    nap::Logger::warn("Modules directory not found: " + modulesDirectory);
                    modulesDirectory = ".";
                }
            }
        }
    }
    
	core.initialize(modulesDirectory);

    std::string rpcServiceTypename = "nap::JsonRpcService";
    RTTI::TypeInfo rpcServiceType = RTTI::TypeInfo::getByName(rpcServiceTypename.c_str());
    if (!rpcServiceType.isValid()) {
        nap::Logger::fatal("Failed to retrieve type: '%s'", rpcServiceTypename.c_str());
        return -1;
    }
    auto rpcService = core.getOrCreateService(rpcServiceType);
    rpcService->getAttribute<bool>("threaded")->setValue(false);
    rpcService->getAttribute<bool>("running")->setValue(true);

	if (keepAlive) {
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(4));
            nap::Logger::info("I'm alive");
		}
	}

	return 0;
}
