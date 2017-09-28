#include <iostream>

// Std includes
#include <thread>

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <utility/dllexport.h>

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
    
    auto audioService = core.getOrCreateService<nap::audio::AudioDeviceService>();
    if (!audioService->init(errorState))
    {
        nap::Logger::fatal(errorState.toString());
        return false;
    }
    
    // Get resource manager service
    resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();
        
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
    
//    auto stepCount = 10;
//    float value = 0;
//    nap::audio::LinearRamper<float> linearRamper(value);
//    nap::audio::ExponentialRamper<float> exponentialRamper(value);
//    nap::audio::EqualPowerTranslator<float> table(16);
////    table.fill([](float x){ return x; });
//
//    linearRamper.ramp(1, stepCount);
//    for (auto i = 0; i < stepCount; i++)
//    {
//        linearRamper.step();
//        std::cout << " " << value << " " << table.translate(value) << std::endl;
//    }
//
//    linearRamper.ramp(0, stepCount);
//    for (auto i = 0; i < stepCount; i++)
//    {
//        linearRamper.step();
//        std::cout << " " << value << " " << table.translate(value) << std::endl;
//    }
    
    

    nap::Core core;

    if (!init(core))
        return -1;

    while (true)
    {
        resourceManagerService->checkForFileChanges();
        resourceManagerService->update();
    }

//    std::cout << "Press return to quit" << std::endl;
//    std::cin.get();
    
	return 0;
}

     
