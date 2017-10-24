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


void update(double deltaTime)
{

}


// Main loop
int main(int argc, char *argv[])
{
    nap::Core core;
    
	// Pointer to function used inside update call by core
	std::function<void(double)> update_call = std::bind(&update, std::placeholders::_1);

	if (!init(core))
		return -1;
    
    while (true)
		core.update(update_call);
    
	return 0;
}

     
