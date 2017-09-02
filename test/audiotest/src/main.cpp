

// Nap includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <utility/dllexport.h>

// Audio module includes
#include <audiodevice.h>
#include <audiotypes.h>
#include <audioservice.h>
#include <utility/audiofilereader.h>
#include <nodes/noise.h>
#include <nodes/bufferplayer.h>
#include <nodes/stereopanner.h>

//////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////

// Nap Objects
nap::ResourceManagerService* resourceManagerService = nullptr;
nap::audio::AudioService* audioService = nullptr;

// Audio device
std::unique_ptr<nap::audio::AudioDeviceManager> audioDeviceManager = nullptr;

// Audio buffer
nap::audio::MultiSampleBuffer audioFileBuffer;

// Audio nodes
std::unique_ptr<nap::audio::Noise> noise = nullptr;
std::unique_ptr<nap::audio::BufferPlayer> playerLeft = nullptr;
std::unique_ptr<nap::audio::BufferPlayer> playerRight = nullptr;
std::unique_ptr<nap::audio::StereoPanner> panner = nullptr;
std::unique_ptr<nap::audio::AudioOutputNode> leftOut = nullptr;
std::unique_ptr<nap::audio::AudioOutputNode> rightOut = nullptr;


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
    float fileSampleRate;
    nap::audio::readAudioFile("data/audiotest/test.wav", audioFileBuffer, fileSampleRate);
    
    audioService = core.getOrCreateService<nap::audio::AudioService>();
    
    // Start audio device
    audioDeviceManager = std::make_unique<nap::audio::AudioDeviceManager>(*audioService);
    audioDeviceManager->startDefaultDevice(1, 2, 44100, 256);
    
    noise = std::make_unique<nap::audio::Noise>(*audioService);
    playerLeft = std::make_unique<nap::audio::BufferPlayer>(*audioService);
    playerRight = std::make_unique<nap::audio::BufferPlayer>(*audioService);
    panner = std::make_unique<nap::audio::StereoPanner>(*audioService);
    
    audioService->execute([&](){
        panner->leftInput.connect(playerLeft->audioOutput);
        panner->rightInput.connect(playerRight->audioOutput);
        panner->setPanning(0.5);
        
        leftOut = std::make_unique<nap::audio::AudioOutputNode>(*audioService);
        leftOut->outputChannel = 0;
        leftOut->audioInput.connect(panner->leftOutput);
        
        rightOut = std::make_unique<nap::audio::AudioOutputNode>(*audioService);
        rightOut->outputChannel = 1;
        rightOut->audioInput.connect(panner->rightOutput);
        
        playerLeft->play(&audioFileBuffer[0], 0, fileSampleRate / audioService->getSampleRate());
        playerRight->play(&audioFileBuffer[1], 0, fileSampleRate / audioService->getSampleRate());
    });
    
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
	run(core);
    
    audioDeviceManager->stop();
    
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
       
     
