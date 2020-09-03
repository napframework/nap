#pragma once

// Std includes
#include <vector>

// RTTI includes
#include <rtti/rtti.h>

// Nap includes
#include <utility/threading.h>

// Audio includes
#include <audio/utility/asyncobserver.h>
#include <audio/utility/audiotypes.h>
#include <audio/utility/safeptr.h>

namespace nap
{
    
    namespace audio
    {
        
        // Forward declarations
        class NodeManager;
        class Node;
        class ParentProcess;
        
        /**
         * A process that is executed once on every audio callback.
         * Base class to @Node.
         */
        class NAPAPI Process
        {
            RTTI_ENABLE()
            
            friend class NodeManager;
            
        public:
            /**
             * Constructor takes the node manager this process runs on.
             */
            Process(NodeManager& nodeManager) : mNodeManager(&nodeManager) { }
            
            /**
             * Constructor that takes a parent process.
             */
            Process(ParentProcess& parent);

            /**
             * We need to delete these so that the compiler doesn't try to use them. Otherwise we get compile errors on vector<unique_ptr>.
             */
            Process(const Process&) = delete;
            Process& operator=(const Process&) = delete;
            
            virtual ~Process() { }
            
            /*
             * Invoked by OutputPin::pull methods and by parent processes.
             * Makes sure all the outputs of this node are being processed up to the current time stamp.
             * Calls process() call if not up to date.
             */
            void update();
            
            /**
             * @return: The node manager that this process is processed on
             */
            NodeManager& getNodeManager() const { return *mNodeManager; }
            
            /**
             * Returns the internal buffersize of the node system that this process belongs to.
             * Output is being pulled through the graph buffers of this size at a time.
             * Not to be confused with the buffersize that the audio callback runs on!
             */
            int getBufferSize() const;
            
            /**
             * Returns the sample rate that the node system runs on
             */
            float getSampleRate() const;
            
            /**
             * Returns the current time in samples
             */
            DiscreteTimeValue getSampleTime() const;
            
        protected:
            /**
             * Called whenever the sample rate that the node system runs on changes.
             * Can be overwritten to respond to changes
             * @param sampleRate: then new sample rate
             */
            virtual void sampleRateChanged(float sampleRate) { }
            
            /**
             * Called whenever the buffersize that the node system runs on changes.
             * Can be overwriten to respond to changes.
             * @param bufferSize: the new buffersize
             */
            virtual void bufferSizeChanged(int bufferSize) { }

        private:
            /**
             * Has to be overwritted by descendants to specify the actual process.
             */
            virtual void process() = 0;
            
            NodeManager* mNodeManager = nullptr; // The node manager that this process is processed on
            DiscreteTimeValue mLastCalculatedSample = 0; // The time stamp of the latest calculated sample by this node
        };
        
        
        /**
         * Parent process that triggers the processing of a number of child processes that it refers to.
         */
        class NAPAPI ParentProcess : public Process
        {
            RTTI_ENABLE(Process)
            
        public:
            enum class Mode { Sequential, Parallel };
            
        public:
            /**
             * Constructor takes the node manager the process runs on, a @ThreadPool and an @AsyncObserver that will be used in case of parallel processing of the child processes.
             */
            ParentProcess(NodeManager& nodeManager, ThreadPool& threadPool, AsyncObserver& observer) : Process(nodeManager), mThreadPool(threadPool), mAsyncObserver(observer) { }
            
            /**
             * Constructor that takes the parent process of this process as argument.
             */
            ParentProcess(ParentProcess& parent) : Process(parent), mThreadPool(parent.mThreadPool), mAsyncObserver(parent.mAsyncObserver) { }
            
            /**
             * Add a reference to a child process whose processing will be triggered by this parent process.
             */
            void addChild(Process& child);
            
            /**
             * Removes the reference of a child process so it will no longer be processed by this parent process.
             */
            void removeChild(Process& child);
            
            /**
             * Directly triggers parallel processing of all child processes.
             */
            void processParallel();
            
            /**
             * Directly triggers sequential (or serial) processing of all child processes.
             */
            void processSequential();
            
            /**
             * Process method chooses between parallel or sequential processing of the children according to mode specified by @setMode().
             */
            void process() override;
            
            /**
             * Specifies wether the child processes will be processed in parallel or sequential.
             */
            void setMode(Mode mode) { mMode.store(mode); }
            
            /**
             * Returns wether the child processes will be processed in parallel or sequential.
             */
            Mode getMode() const { return mMode.load(); }
            
        private:
            ThreadPool& mThreadPool;
            AsyncObserver& mAsyncObserver;
            std::vector<Process*> mChildren;
            std::atomic<Mode> mMode = { Mode::Sequential };
        };
        
        

    }
    
}
