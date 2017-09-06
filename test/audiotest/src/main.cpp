#include <iostream>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <utility/dllexport.h>

// Audio module includes
#include <audiodevice.h>
#include <audiotypes.h>

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
    
    // Get resource manager service
    resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();
    
    
    auto audioService = core.getOrCreateService<nap::audio::AudioService>();
    if (!audioService->init(errorState))
    {
        nap::Logger::fatal(errorState.toString());
        return false;
    }
    
    // Load scene
    if (!resourceManagerService->loadFile("data/audiotest/audiotest.json", errorState))
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
    
//    while (true)
//    {
//        resourceManagerService->checkForFileChanges();
//        resourceManagerService->update();
//    }

    std::cout << "Press return to quit" << std::endl;
    std::cin.get();
    
	return 0;
}

     
