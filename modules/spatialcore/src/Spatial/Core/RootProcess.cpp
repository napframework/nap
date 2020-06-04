#include "RootProcess.h"

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/node/buffernode.h>

// Nap includes
#include <utility/threading.h>


namespace nap
{
    
    namespace spatial
    {
        
        RootProcess::RootProcess(audio::AudioService& audioService, int threadCount) : Process(audioService.getNodeManager())
        {
            mThreadPool = std::make_unique<ThreadPool>(threadCount, 64, true);
            auto& nodeManager = audioService.getNodeManager();
            mInputLevelMeterProcess = nodeManager.makeSafe<audio::ParentProcess>(nodeManager, *mThreadPool, mAsyncObserver);
            mInputProcess = nodeManager.makeSafe<audio::ParentProcess>(nodeManager, *mThreadPool, mAsyncObserver);
            mParticleProcess = nodeManager.makeSafe<audio::ParentProcess>(audioService.getNodeManager(), *mThreadPool, mAsyncObserver);
            mPreParticleProcess = nodeManager.makeSafe<audio::ParentProcess>(audioService.getNodeManager(), *mThreadPool, mAsyncObserver);
            mPostParticleProcess = nodeManager.makeSafe<audio::ParentProcess>(audioService.getNodeManager(), *mThreadPool, mAsyncObserver);
            mOutputProcess = nodeManager.makeSafe<audio::ParentProcess>(nodeManager, *mThreadPool, mAsyncObserver);
            mPostMixdownProcess = nodeManager.makeSafe<audio::ParentProcess>(nodeManager, *mThreadPool, mAsyncObserver);

            nodeManager.registerRootProcess(*this);
        }
        
        
        RootProcess::~RootProcess()
        {
            getNodeManager().unregisterRootProcess(*this);
        }
        
        
        void RootProcess::process()
        {
            if (mInputsParallel.load())
            {
                mInputLevelMeterProcess->processParallel();
                mInputProcess->processParallel();
            }
            else {
                mInputLevelMeterProcess->processSequential();
                mInputProcess->processSequential();
            }

            mPreParticleProcess->processSequential();
            if (mParticlesParallel.load())
                mParticleProcess->processParallel();
            else
                mParticleProcess->processSequential();
            mPostParticleProcess->processSequential();

            mOutputProcess->processSequential();
            
            for (auto mixdownBuffer : mMixdownBuffers)
                mixdownBuffer->update();
            
            mPostMixdownProcess->processSequential();
        }   
        
        
        void RootProcess::registerMixdownBuffer(audio::BufferNode& buffer)
        {
            auto bufferPtr = &buffer;
            getNodeManager().enqueueTask([&, bufferPtr](){
                auto it = std::find(mMixdownBuffers.begin(), mMixdownBuffers.end(), bufferPtr);
                if (it == mMixdownBuffers.end())
                    mMixdownBuffers.emplace_back(bufferPtr);
            });

        }
        
        
        void RootProcess::unregisterMixdownBuffer(audio::BufferNode& buffer)
        {
            auto bufferPtr = &buffer;
            getNodeManager().enqueueTask([&, bufferPtr](){
                auto it = std::find(mMixdownBuffers.begin(), mMixdownBuffers.end(), bufferPtr);
                if (it != mMixdownBuffers.end())
                    mMixdownBuffers.erase(it);
            });
        }
        
        
        void RootProcess::setThreadCount(int value)
        {
            auto threadCount = value;
            getNodeManager().enqueueTask([&, threadCount](){
                mThreadPool->resize(threadCount);
            });
        }

    }
    
}
