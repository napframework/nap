#include "audionodemanager.h"
#include "audionode.h"
#include "audionodeptr.h"

#include <nap/logger.h>
#include <nap/core.h>

namespace nap
{
    
    namespace audio
    {

        void NodeManager::process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer)
        {
            // clean the output buffers
            for (auto channel = 0; channel < mOutputChannelCount; ++channel)
                memset(outputBuffer[channel], 0, sizeof(float) * framesPerBuffer);
            
            // process tasks that are enqueued from outside the audio thread
            mAudioCallbackTaskQueue.process();
            
            // clean the Node trash bin with nodes that are no longer used and scheduled for destruction
            clearNodeTrashBin();
            
            mInputBuffer = inputBuffer;
            
            mInternalBufferOffset = 0;
            while (mInternalBufferOffset < framesPerBuffer)
            {
                for (auto& channelMapping : mOutputMapping)
                    channelMapping.clear();
                
                {
                    for (auto& root : mRootNodes)
                        root->process();
                }
                
                for (auto channel  = 0; channel < mOutputChannelCount; ++channel)
                {
                    for (auto& output : mOutputMapping[channel])
                        for (auto j = 0; j < mInternalBufferSize; ++j)
                            outputBuffer[channel][mInternalBufferOffset + j] += (*output)[j];
                }
                
                mInternalBufferOffset += mInternalBufferSize;
                mSampleTime += mInternalBufferSize;
            }
        }
        
        
        void NodeManager::setInputChannelCount(int inputChannelCount)
        {
            mInputChannelCount = inputChannelCount;
        }
        
        
        
        void NodeManager::setOutputChannelCount(int outputChannelCount)
        {
            mOutputChannelCount = outputChannelCount;
            mOutputMapping.clear();
            mOutputMapping.resize(mOutputChannelCount);
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
            mNodes.emplace(&node);
            node.setSampleRate(mSampleRate);
            node.setBufferSize(mInternalBufferSize);
        }
        
        
        void NodeManager::unregisterNode(Node& node)
        {
            mNodes.erase(&node);
        }
        
        
        void NodeManager::registerRootNode(Node& rootNode)
        {
            auto rootNodePtr = &rootNode;
            execute([&, rootNodePtr](){ mRootNodes.emplace(rootNodePtr); });
        }

        
        void NodeManager::unregisterRootNode(Node& rootNode)
        {
            auto rootNodePtr = &rootNode;
            execute([&, rootNodePtr](){ mRootNodes.erase(rootNodePtr); });
        }

        
        void NodeManager::provideOutputBufferForChannel(SampleBufferPtr buffer, int channel)
        {
            assert(channel < mOutputMapping.size());
            mOutputMapping[channel].emplace_back(buffer);
        }
        
        
        void NodeManager::clearNodeTrashBin()
        {
            std::unique_ptr<Node> node = nullptr;
            while (mNodeTrashBin.try_dequeue(node))
            {
                // this call is rather symbolic, as the dequeued node will go out of scope anyway and will be implicitly destructed.
                node = nullptr;
                Logger::debug("removing node from trash");
            }
        }

        
    }
    
}

