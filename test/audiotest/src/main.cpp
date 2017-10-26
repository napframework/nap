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
bool init(nap::Core& core, nap::utility::ErrorState& error)
{    
	// Initialize the engine -> loads all modules
	if (!core.initializeEngine(error))
	{
		nap::Logger::fatal("Unable to initialize engine: %s", error.toString().c_str());
		return false;
	}

    // Load scene
	resourceManager = core.getResourceManager();
	if (!resourceManager->loadFile("data/audiotest/audiotest.json", error))
    {
        nap::Logger::fatal("Unable to deserialize resources: \n %s", error.toString().c_str());
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

	nap::utility::ErrorState error;
	if (!init(core, error))
		return -1;
    
	// Signal Start
	core.start();

    while (true)
		core.update(update_call);
    
	// Shutdown core
	core.shutdown();

	return 0;
}

     
