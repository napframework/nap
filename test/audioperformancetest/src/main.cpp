#include <iostream>

// Std includes
#include <thread>

#include <node/oscillatornode.h>
#include <portaudio.h>
#include <service/audiodeviceservice.h>

/*
Test application to compare portaudio performance between different builds, platforms and architectures.
Plays a number of oscillators in the audio callback.
*/

constexpr float sampleRate = 44100;
constexpr int bufferSize = 512;
constexpr int oscillatorAmount = 1500;

struct CallbackData {
	std::vector<std::unique_ptr<nap::audio::OscillatorNode>> oscillators;
	nap::audio::NodeManager* nodeManager;
};

static int audioCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	float** out = (float**)outputBuffer;
	float** in = (float**)inputBuffer;

	auto data = reinterpret_cast<CallbackData*>(userData);
	data->nodeManager->process(in, out, framesPerBuffer);
	for (auto channel = 0; channel < 2; channel++)
	{

		for (auto i = 0; i < framesPerBuffer; ++i)
			out[channel][i] = 0;

		for (auto& osc : data->oscillators)
		{
			auto oscOutput = osc->output.pull();
			for (auto i = 0; i < framesPerBuffer; ++i)
			{
				out[channel][i] += (*oscOutput)[i] / float(data->oscillators.size());
			}
		}
	}

	return 0;
}

/*
* This routine is called by portaudio when playback is done.
*/
static void StreamFinished(void* userData)
{
}

// Main loop
int main(int argc, char *argv[])
{
	PaStreamParameters outputParameters;
	PaStream *stream;
	PaError err;

	nap::audio::WaveTable waveTable(2048);
	nap::audio::NodeManager nodeManager;
	nodeManager.setSampleRate(sampleRate);
	nodeManager.setInternalBufferSize(bufferSize);
	CallbackData data;
	data.nodeManager = &nodeManager;
	for (auto i = 0; i < oscillatorAmount; ++i)
		data.oscillators.emplace_back(std::make_unique<nap::audio::OscillatorNode>(nodeManager, waveTable));

	err = Pa_Initialize();
	if (err != paNoError) goto error;

	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default output device.\n");
		goto error;
	}
	outputParameters.channelCount = 2; /* stereo */
	outputParameters.sampleFormat = paFloat32 | paNonInterleaved; /* 32 bit floating point output */
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

	err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
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

     
