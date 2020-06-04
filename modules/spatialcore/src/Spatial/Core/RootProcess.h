 #pragma once

// Audio includes
#include <audio/core/process.h>
#include <audio/node/buffernode.h>


namespace nap
{

    class ThreadPool;
    class AsyncObserver;
    namespace audio
    {
        class AudioService;
    }

    
    namespace spatial
    {
        
        /**
         * The root DSP process that takes care of parallelizing all the DSP processing in the DSP node graph.
         */
        class NAPAPI RootProcess : public audio::Process
        {
        public:
            /**
             * Constructor takes the @NodeManeger the process runs on and a default size of the threadpool./
             */
            RootProcess(audio::AudioService& audioService, int threadCount = 7);
            ~RootProcess() override;
            
            void process() override;
            
            /**
             * Register a VU level meter node for processing. Used by @InputLevelMeter.
             */
            void registerInputLevelMeter(audio::Process& levelMeter) { mInputLevelMeterProcess->addChild(levelMeter); }
            
            /**
             * Unregister a VU level meter node. Used by @InputLevelMeter.
             */
            void unregisterInputLevelMeter(audio::Process& levelMeter) { mInputLevelMeterProcess->removeChild(levelMeter); }
            
            /**
             * Register an input process for processing. Used by @SpatialSource.
             */
            void registerSource(audio::Process& source) { mInputProcess->addChild(source); }
            
            /**
             * Unregister an input process for processing. Used by @SpatialSource.
             */
            void unregisterSource(audio::Process& source) { mInputProcess->removeChild(source); }
            
            /**
             * Register a particle to be processed.
             */
            void registerParticle(audio::Process& particle) { mParticleProcess->addChild(particle); }
            
            /**
             * Unregister a particle to be processed.
             */
            void unregisterParticle(audio::Process& particle) { mParticleProcess->removeChild(particle); }

            /**
             * Register a process to be processed before the particles will be processed.
             */
            void registerPreParticleProcess(audio::Process& process) { mPreParticleProcess->addChild(process); }

            /**
             * Unregister a a process to be processed before the particles will ne processed.
             */
            void unregisterPreParticleProcess(audio::Process& process) { mPreParticleProcess->removeChild(process); }
            /**
             * Register a process to be processed after the particles have been processed. For example, circular buffers will be processed in an extra pass after the particles have been processed.
             */
            void registerPostParticleProcess(audio::Process& process) { mPostParticleProcess->addChild(process); }

            /**
             * Unregister a a process to be processed after the particles have been processed.
             */
            void unregisterPostParticleProcess(audio::Process& process) { mPostParticleProcess->removeChild(process); }

            /**
             * Used by a @SpeakerSetup to register it's processing.
             */
            void registerSpeakerSetup(audio::Process& speakerSetup) { mOutputProcess->addChild(speakerSetup); }
            
            /**
             * Used by a @SpeakerSetup to unregister it's processing.
             */
            void unregisterSpeakerSetup(audio::Process& speakerSetup) { mOutputProcess->removeChild(speakerSetup); }
            
            /**
             * Registers a @BufferNode to be updated at the end of the root process.
             * Used for one buffer delay to enable feedback and routing from one sound object to another.
             */
            void registerMixdownBuffer(audio::BufferNode& buffer);
            
            /**
             * Unregisters a @BufferNode to be updated at the end of the root process.
             * Used for one buffer delay to enable feedback and routing from one sound object to another.
             */
            void unregisterMixdownBuffer(audio::BufferNode& buffer);
            
            /**
             * Register a process to be processed after the mixdown buffers have been processed.              
             */
            void registerPostMixdownProcess(audio::Process& process) { mPostMixdownProcess->addChild(process); }
            
            /**
             * Unregister a a process to be processed after the mixdown buffers have been processed.
             */
            void unregisterPostMixdownProcess(audio::Process& process) { mPostMixdownProcess->removeChild(process); }
            
            /**
             * Specify wether the inputs should be processed in parallel.
             */
            void setInputsParallel(bool value) { mInputsParallel.store(value); }
            
            /**
             * @return: wether the inputs should be processed in parallel.
             */
            bool getInputsParallel() const { return mInputsParallel.load(); }
            
            /**
             * Specify wether the particles will be processed in parallel.
             */
            void setParticlesParallel(bool value) { mParticlesParallel.store(value); }
            
            /**
             * @return: wether the particles will be processed in parallel.
             */
            bool getParticlesParallel() const { return mParticlesParallel.load(); }
            
            /**
             * Set the size of the threadpool used to parallelize tasks.
             */
            void setThreadCount(int value);
            
            /**
             * @return: the size of the threadpool used to parallelize tasks.
             */
            int getThreadCount() const { return mThreadPool->getThreadCount(); }
            
            ThreadPool& getThreadPool() { return *mThreadPool; }
            audio::AsyncObserver& getAsyncObserver() { return mAsyncObserver; }

        private:
            std::unique_ptr<ThreadPool> mThreadPool = nullptr;
            audio::AsyncObserver mAsyncObserver;

            audio::SafeOwner<audio::ParentProcess> mInputLevelMeterProcess = nullptr;
            audio::SafeOwner<audio::ParentProcess> mInputProcess = nullptr;
            audio::SafeOwner<audio::ParentProcess> mPreParticleProcess = nullptr;
            audio::SafeOwner<audio::ParentProcess> mParticleProcess = nullptr;
            audio::SafeOwner<audio::ParentProcess> mPostParticleProcess = nullptr;
            audio::SafeOwner<audio::ParentProcess> mOutputProcess = nullptr;
            std::vector<audio::BufferNode*> mMixdownBuffers;
            audio::SafeOwner<audio::ParentProcess> mPostMixdownProcess = nullptr;
            
            std::atomic<bool> mInputsParallel = { true };
            std::atomic<bool> mParticlesParallel = { true };
        };
        
    }
    
}
