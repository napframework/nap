/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audionodemanager.h"
#include "audionode.h"

#include <nap/logger.h>
#include <nap/core.h>

namespace nap
{
	namespace audio
	{
		
		NodeManager::~NodeManager()
		{
			// Tell the nodes that their node manager is outta here, so they won't try to unregister themselves in their dtors.
			for (auto& node : mNodes)
			{
				node->mNodeManager = nullptr;
				node->mRegisteredWithNodeManager.store(false);
			}
		}
		
		
		void NodeManager::process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
		{
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
			if (!result)
				Logger::warn("NodeManager: Failed to enqueue task");
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
			for (auto& node : mNodes)
				node->setBufferSize(size);
		}
		
		
		void NodeManager::setSampleRate(float sampleRate)
		{
			mSampleRate = sampleRate;
			mSamplesPerMillisecond = sampleRate / 1000.;
			for (auto& node : mNodes)
				node->setSampleRate(sampleRate);
		}
		
		
		void NodeManager::registerNode(Node& node)
		{
			node.setSampleRate(mSampleRate);
			node.setBufferSize(mInternalBufferSize);
			auto oldSampleRate = mSampleRate;
			auto oldBufferSize = mInternalBufferSize;
			enqueueTask([&, oldSampleRate, oldBufferSize]() {
				// In the extremely rare case the buffersize or the samplerate of the node manager have been changed in between the enqueueing of the task and its execution on the audio thread, we set them again.
				// However we prefer not to, in order to avoid memory allocation on the audio thread.
				if (oldSampleRate != mSampleRate)
					node.setSampleRate(mSampleRate);
				if (oldBufferSize != mInternalBufferSize)
					node.setBufferSize(mInternalBufferSize);
				node.mRegisteredWithNodeManager.store(true);
				mNodes.emplace(&node);
			});
		}
		
		
		void NodeManager::unregisterNode(Node& node)
		{
			mNodes.erase(&node);
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

