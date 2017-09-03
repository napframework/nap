#include <iostream>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <utility/dllexport.h>

// Audio module includes
#include <audioservice.h>
#include <audiotypes.h>

nap::ResourceManagerService* resourceManagerService = nullptr;


/**
* Initialize all the resources and instances
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core)
{
    core.initialize();
    
    // Get resource manager service
    resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();
    
    // Collects all the errors
    nap::utility::ErrorState errorState;
    
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
    
    while (true)
    {
        resourceManagerService->checkForFileChanges();
        resourceManagerService->update();
    }

//    std::cout << "Hit key to quit" << std::endl;
//    std::cin.get();
    
	return 0;
}

     
