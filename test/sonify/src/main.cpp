#include <iostream>

// Std includes
#include <thread>

// Nap includes
#include <utility/dllexport.h>
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>

#include <renderservice.h>
#include <oscservice.h>

// Audio module includes
#include <service/audiodeviceservice.h>
#include <utility/audiotypes.h>
#include <utility/linearramper.h>
#include <utility/exponentialramper.h>
#include <utility/translator.h>

nap::ResourceManagerService* resourceManagerService = nullptr;


/**
* Initialize all the resources and instances
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core)
{
    // Collects all the errors
    nap::utility::ErrorState errorState;
    
    core.initialize();
    
    core.getOrCreateService<nap::OSCService>();
    
    auto audioService = core.getOrCreateService<nap::audio::AudioDeviceService>();
    if (!audioService->init(errorState))
    { 
        nap::Logger::fatal(errorState.toString());
        return false;
    }
    
    // Get resource manager service
    resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();
    
    auto renderService = core.getOrCreateService<nap::RenderService>();
    nap::utility::ErrorState error;
    if (!renderService->init(error))
    {
        nap::Logger::fatal(error.toString());
        return false;
    }
    
    nap::Logger::info("initialized render service: %s", renderService->getTypeName().c_str());

    // Load scene
    if (!resourceManagerService->loadFile("data/sonify/sonify.json", errorState))
    {
        nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
        return false;
    } 

	return true;
}

// Main loop
int main(int argc, char *argv[])
{
    
    nap::Core core;

    if (!init(core))
        return -1;

    while (true)
    {
        resourceManagerService->checkForFileChanges();
        resourceManagerService->update();
        int ns = 0.5 * 1000000;
        std::this_thread::sleep_for(std::chrono::nanoseconds(ns));
    }

//    std::cout << "Press return to quit" << std::endl;
//    std::cin.get();
    
	return 0;
}

     
