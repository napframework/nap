

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>

// Audio module includes
#include <audiodevice.h>
#include <audiotypes.h>
#include <utility/audiofilereader.h>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects
nap::ResourceManagerService* resourceManagerService = nullptr;

// Audio device
nap::audio::AudioDeviceManager audioDeviceManager;

// Audio buffer
nap::audio::MultiSampleBuffer audioFileBuffer;

//////////////////////////////////////////////////////////////////////////

// Some utilities
void run(nap::Core& core);


// Called when the window is updating
void onUpdate()
{
	// Update all resources
	resourceManagerService->update();
}



/**
* Initialize all the resources and instances
* slowly migrating all functionality to nap
*/
bool init(nap::Core& core)
{
    // Read an audio file
    float sampleRate;
    nap::audio::readAudioFile("data/audiotest/test.wav", audioFileBuffer, sampleRate);
    
    // Start audio device
    audioDeviceManager.startDefaultDevice(1, 2, 44100, 256);
    
	core.initialize();

	// Get resource manager service
	resourceManagerService = core.getOrCreateService<nap::ResourceManagerService>();

	// Collects all the errors
	nap::utility::ErrorState errorState;

	// Load scene
//	if (!resourceManagerService->loadFile("data/audiotest/audiotest.json", errorState))
//	{
//		nap::Logger::fatal("Unable to deserialize resources: \n %s", errorState.toString().c_str());
//		return false;        
//	} 
	
//	// Get important entities
//	cameraEntity = resourceManagerService->findEntity("CameraEntity");
//	assert(cameraEntity != nullptr);
//
//	// Store all render windows
//	renderWindows.push_back(resourceManagerService->findObject<nap::RenderWindow>("Window"));
//
//	// Store laser dacs
//	laser_one = resourceManagerService->findObject<nap::EtherDreamDac>("Laser1");
//	laserEntity = resourceManagerService->findEntity("LaserEntity1");
//
//	// Set render states
//	nap::RenderState& render_state = renderService->getRenderState();
//	render_state.mEnableMultiSampling = true;
//	render_state.mLineWidth = 1.3f;
//	render_state.mPointSize = 2.0f;
//	render_state.mPolygonMode = opengl::PolygonMode::FILL;

	return true;
}

// Main loop
int main(int argc, char *argv[])
{
	// Create core
	nap::Core core;

	// Initialize render stuff
	if (!init(core))
		return -1;

	// Run loop
//	run(core);
    
    audioDeviceManager.stop();
    
	return 0;
}

void run(nap::Core& core)
{
	// Run function
	bool loop = true;


	// Loop
	while (loop)
	{
		// run update call
		onUpdate();
	}

}
       
     
