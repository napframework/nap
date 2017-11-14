#include <iostream>

// Std includes
#include <thread>

#include <node/oscillatornode.h>
#include <portaudio.h>
#include <service/AudioService.h>

/*
Test application to compare portaudio performance between different builds, platforms and architectures.
Plays a specified number of oscillators in the audio callback for profiling purposes.
You should hear a clear test tone if the system can handle the amount of oscillators specified.
 
Fill in the constants below:
*/
constexpr float sampleRate = 44100;
constexpr int bufferSize = 512;
constexpr int oscillatorAmount = 256;

// User data passed to the audio callback
struct CallbackData {
	std::vector<std::unique_ptr<nap::audio::OscillatorNode>> oscillators;
	nap::audio::NodeManager* nodeManager;
};


// The audio callback
static int audioCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	float** out = (float**)outputBuffer;
	float** in = (float**)inputBuffer;

	auto data = reinterpret_cast<CallbackData*>(userData);
	data->nodeManager->process(in, out, framesPerBuffer); // this is a dummy call and just proceeds the clock by one buffer
	for (auto channel = 0; channel < 2; channel++)
	{
        // Clear output buffer
		for (auto i = 0; i < framesPerBuffer; ++i)
			out[channel][i] = 0;

        // Process all the oscillators directly in order to by-pass the node system
		for (auto& osc : data->oscillators)
		{
            // Add one oscillator's output to the output buffer
			auto oscOutput = osc->output.pull();
			for (auto i = 0; i < framesPerBuffer; ++i)
				out[channel][i] += (*oscOutput)[i] / float(data->oscillators.size());
		}
	}

	return 0;
}


// Main loop
int main(int argc, char *argv[])
{
	PaStreamParameters outputParameters;
	PaStream *stream;
	PaError err;

    
    nap::audio::NodeManager nodeManager; // we need a dummy node manager to provide global settings and a clock
	nap::audio::WaveTable waveTable(2048); // a sinewave table for the oscillators
	nodeManager.setSampleRate(sampleRate);
	nodeManager.setInternalBufferSize(bufferSize);
    
	CallbackData data;
	data.nodeManager = &nodeManager;
    
    // allocate and init the oscillators
	for (auto i = 0; i < oscillatorAmount; ++i)
		data.oscillators.emplace_back(std::make_unique<nap::audio::OscillatorNode>(nodeManager, waveTable));

	err = Pa_Initialize();
	if (err != paNoError) goto error;

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default output device.\n");
		goto error;
	}
	outputParameters.channelCount = 2; // Stereo
	outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		sampleRate,
		bufferSize,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		audioCallback,
		&data);
	if (err != paNoError) goto error;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto error;

	printf("Play for %d seconds.\n", 60);
	Pa_Sleep(60 * 1000);

	err = Pa_StopStream(stream);
	if (err != paNoError) goto error;

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto error;

	Pa_Terminate();
	printf("Test finished.\n");

	return err;
error:
	Pa_Terminate();
	fprintf(stderr, "An error occured while using the portaudio stream\n");
	fprintf(stderr, "Error number: %d\n", err);
	fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
	return err;
}

     
