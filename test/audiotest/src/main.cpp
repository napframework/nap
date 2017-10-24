#include <iostream>

// Std includes
#include <thread>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <utility/dllexport.h>

// Audio module includes
#include <audiodevice.h>
#include <audiotypes.h>

nap::ResourceManager* resourceManager = nullptr;


/**
* Initialize all the resources and instances
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core)
{    
	// Initialize the engine -> loads all modules
    core.initializeEngine();
    
	// Create services
    auto audioService = core.getOrCreateService<nap::audio::AudioService>();
 
	// Initialize all services
	nap::utility::ErrorState errorState;
	if (!core.initializeServices(errorState))
	{
		nap::Logger::fatal("unable to initialize services: %s", errorState.toString().c_str());
		return false;
	}

    // Load scene
	resourceManager = core.getResourceManager();
	if (!resourceManager->loadFile("data/audiotest/audiotest.json", errorState))
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
        resourceManager->checkForFileChanges();
        resourceManager->update();
//        std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
    }

//    std::cout << "Press return to quit" << std::endl;
//    std::cin.get();
    
	return 0;
}

     
