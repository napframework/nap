/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audionodemanager.h"
#include "audionode.h"

#include <nap/logger.h>
#include <nap/core.h>

#ifndef RASPBERRY
    #include <xmmintrin.h>
#endif

namespace nap
{
	namespace audio
	{

		NodeManager::~NodeManager()
		{
			// Tell the nodes that their node manager is outta here, so they won't try to unregister themselves in their dtors.
			for (auto& process : mProcesses)
			{
				process->mNodeManager = nullptr;
                process->mRegisteredWithNodeManager.store(false);
			}
		}


		void NodeManager::process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
		{
#ifndef RASPBERRY
			// Disable denormal math
            // Denormal math can have a dramatic impact on the CPU load for any DSP algorithm that contains a feedback loop.
            // However denormal precision doesn't produce any audible result to the human ear.
            // Unfortunately this code is not supported on ARM32 processors though, hence the #ifndef .
			int oldMXCSR = _mm_getcsr();
			int newMXCSR = oldMXCSR | 0x8040;
			_mm_setcsr( newMXCSR);
#endif

			// clean the output buffers
			for (auto channel = 0; channel < mOutputChannelCount; ++channel)
				memset(outputBuffer[channel], 0, sizeof(float) * framesPerBuffer);

			for (auto channel = 0; channel < mInputChannelCount; ++channel)
				mInputBuffer[channel] = inputBuffer[channel];

			mInternalBufferOffset = 0;
			while (mInternalBufferOffset < framesPerBuffer)
			{
				mTaskQueue.process();
				for (auto& channelMapping : mOutputMapping)
					channelMapping.clear();

				{
					for (auto& root : mRootProcesses)
						root->process();
				}

				for (auto channel = 0; channel < mOutputChannelCount; ++channel) {
					for (auto& output : mOutputMapping[channel])
						for (auto j = 0; j < mInternalBufferSize; ++j)
							outputBuffer[channel][mInternalBufferOffset + j] += (*output)[j];
				}

				mInternalBufferOffset += mInternalBufferSize;
				mSampleTime += mInternalBufferSize;

				mUpdateSignal(mSampleTime);
			}

			if (mInternalBufferOffset != framesPerBuffer)
				nap::Logger::warn("Internal buffer does not fit PortAudio buffer");

#ifndef RASPBERRY
            // Reset previous denormal handling mode
			_mm_setcsr(oldMXCSR);
#endif
		}


		void NodeManager::process(std::vector<SampleBuffer*>& inputBuffer, std::vector<SampleBuffer*>& outputBuffer,
		                          unsigned long framesPerBuffer)
		{
			// clean the output buffers
			for (auto channel = 0; channel < mOutputChannelCount; ++channel)
				memset(outputBuffer[channel]->data(), 0, sizeof(float) * framesPerBuffer);

			for (auto channel = 0; channel < mInputChannelCount; ++channel)
				mInputBuffer[channel] = inputBuffer[channel]->data();

			mInternalBufferOffset = 0;
			while (mInternalBufferOffset < framesPerBuffer)
			{
				mTaskQueue.process();
				for (auto& channelMapping : mOutputMapping)
					channelMapping.clear();

				{
					for (auto& root : mRootProcesses)
						root->update();
				}

				for (auto channel = 0; channel < mOutputChannelCount; ++channel) {
					for (auto& output : mOutputMapping[channel])
						for (auto j = 0; j < mInternalBufferSize; ++j)
							(*outputBuffer[channel])[mInternalBufferOffset + j] += (*output)[j];
				}

				mInternalBufferOffset += mInternalBufferSize;
				mSampleTime += mInternalBufferSize;

				mUpdateSignal(mSampleTime);
			}

			if (mInternalBufferOffset != framesPerBuffer)
				nap::Logger::warn("Internal buffer does not fit PortAudio buffer");
		}


		void NodeManager::enqueueTask(nap::TaskQueue::Task task)
		{
			auto result = mTaskQueue.enqueue(task);
			assert(result);
		}


		void NodeManager::setInputChannelCount(int inputChannelCount)
		{
			mInputChannelCount = inputChannelCount;
			mInputBuffer.resize(inputChannelCount);
			mChannelCountChangedSignal(*this);
		}


		void NodeManager::setOutputChannelCount(int outputChannelCount)
		{
			mOutputChannelCount = outputChannelCount;
			mOutputMapping.clear();
			mOutputMapping.resize(mOutputChannelCount);
			mChannelCountChangedSignal(*this);
		}


		void NodeManager::setInternalBufferSize(int size)
		{
			mInternalBufferSize = size;
			for (auto& process : mProcesses)
				process->setBufferSize(size);
		}


		void NodeManager::setSampleRate(float sampleRate)
		{
			mSampleRate = sampleRate;
			mSamplesPerMillisecond = sampleRate / 1000.;
			for (auto& process : mProcesses)
				process->setSampleRate(sampleRate);
		}


		void NodeManager::registerProcess(Process& process)
		{
			process.setSampleRate(mSampleRate);
			process.setBufferSize(mInternalBufferSize);
			auto oldSampleRate = mSampleRate;
			auto oldBufferSize = mInternalBufferSize;
			enqueueTask([&, oldSampleRate, oldBufferSize]() {
				// In the extremely rare case the buffersize or the samplerate of the node manager have been changed in between the enqueueing of the task and its execution on the audio thread, we set them again.
				// However we prefer not to, in order to avoid memory allocation on the audio thread.
				if (oldSampleRate != mSampleRate)
					process.setSampleRate(mSampleRate);
				if (oldBufferSize != mInternalBufferSize)
					process.setBufferSize(mInternalBufferSize);
				process.mRegisteredWithNodeManager.store(true);
				mProcesses.emplace(&process);
			});
		}


		void NodeManager::unregisterProcess(Process& process)
		{
			mProcesses.erase(&process);
		}


		void NodeManager::registerRootProcess(Process& rootProcess)
		{
			enqueueTask([&]() { mRootProcesses.emplace(&rootProcess); });
		}


		void NodeManager::unregisterRootProcess(Process& rootProcess)
		{
			mRootProcesses.erase(&rootProcess);
		}


		void NodeManager::provideOutputBufferForChannel(SampleBuffer* buffer, int channel)
		{
			if (channel < mOutputMapping.size())
				mOutputMapping[channel].emplace_back(buffer);
		}


	}
}
