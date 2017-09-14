#pragma once

// Std includes
#include <mutex>

// Nap includes
#include <nap/threading.h>

// Audio includes
#include <utility/audiotypes.h>

namespace nap {
    
    namespace audio {
    
        // Forward declarations
        class Node;
        class AudioTrigger;
        
        /**
         * De audio node manager represents a node system for audio processing.
         * The nodes in the system can have multiple inputs and outputs that can be connected between different nodes.
         * A connection represents a mono audio signal.
         * Does not own the nodes but maintains a list of existing nodes that is updated from the node's constructor end destructors.
         */
        class NAPAPI NodeManager
        {
            friend class Node;
            friend class AudioTrigger;
            friend class OutputNode;
            friend class InputNode;
            
        public:
            using OutputMapping = std::vector<std::vector<SampleBufferPtr>>;
            
        public:
            NodeManager() = default;
            
            /**
             * This function is typically called by an audio callback to perform all the audio processing.
             * It feeds the inputbuffer to the audio input nodes and polls the audio output nodes for output.
             * @param inputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param outputBuffer: an array of float arrays, representing one sample buffer for every channel
             * @param framesPerBuffer: the number of samples that has to be processed per channel
             */
            void process(float** inputBuffer, float** outputBuffer, unsigned long framesPerBuffer);
            
            /**
             * @return: the number of input channels that will be fed into the node system
             */
            int getInputChannelCount() const { return mInputChannelCount; }
            
            /**
             * @return: the number of output channels that will be processed by the node system
             */
            int getOutputChannelCount() const { return mOutputChannelCount; }
            
            /**
             * @return: the sample rate that the node system runs on
             */
            float getSampleRate() const { return mSampleRate; }
            
            float getSamplesPerMillisecond() const { return mSampleRate / 1000.f; }
            
            /**
             * Returns the buffer size the node system is running on.
             * Beware: this can be smaller than the buffersize the audio device is running on.
             * The latter is specified by the framesPerBuffer parameter in the process() function.
             */
            int getInternalBufferSize() const { return mInternalBufferSize; }
            
            /**
             * Returns the absolute time in samples
             */
            const DiscreteTimeValue& getSampleTime() const { return mSampleTime; }
            
            /**
             * Sets the number of input channels that will be fed into the node system
             */
            void setInputChannelCount(int inputChannelCount);
            
            /**
             * Sets the number of output channels that will be processed by the node system
             */
            void setOutputChannelCount(int outputChannelCount);
            
            /**
             * Changes the sample rate the node system is running on
             */
            void setSampleRate(float sampleRate);
            
            
            /**
             * Changes the internal buffer size that the node system uses.
             * Beware: this can be smaller than the buffersize the audio device is running on.
             */
            void setInternalBufferSize(int size);
            
            /**
             * Enqueue a task to be executed within the process() method for thread safety
             */
            void execute(TaskQueue::Task task) { mAudioCallbackTaskQueue.enqueue(task); }
            
        private:
            // Used by the nodes to register themselves on construction
            void registerNode(Node& node);
            
            // Used by the nodes to unregister themselves on destrction
            void unregisterNode(Node& node) { mNodes.erase(&node); }
            
            // Used by nodes to register themselves to be processed directly by the node manager
            void registerRootNode(Node& rootNode) { mRootNodes.emplace(&rootNode); }
            
            // Used by nodes to unregister themselves to be processed directly by the node manager
            void unregisterRootNode(Node& rootNode) { mRootNodes.erase(&rootNode); }
            
        private:
            /*
             * Used by @OutputNode to provide new output for the node system
             * Note: multiple output buffers can be provided for the same channel by different output nodes.
             * @param buffer: a buffer of output
             * @param channel: the channel that the output will be played on
             */
            void provideOutputBufferForChannel(SampleBufferPtr buffer, int channel);
            
            /*
             * Used by @InputNode to request audio input for the current buffer
             * @param channel: the input channel that is being monitored
             * @param index: the index of the requested sample within the buffer currently being calculated.
             * TODO: optimize by returning a float array for the whole buffer
             */
            const SampleValue& getInputSample(int channel, int index) const { return mInputBuffer[channel][mInternalBufferOffset + index]; }
            
            int mInputChannelCount = 0;
            int mOutputChannelCount = 0;
            float mSampleRate = 0;
            int mInternalBufferSize = 64;
            
            DiscreteTimeValue mSampleTime = 0;
            unsigned int mInternalBufferOffset = 0;
            
            // for each output channel a vector of buffers that need to be played back on the corresponding channel
            OutputMapping mOutputMapping;
            
            float** mInputBuffer = nullptr;
            
            std::set<Node*> mNodes;
            std::set<Node*> mRootNodes;
            
            nap::TaskQueue mAudioCallbackTaskQueue;
        };
        
    }
}
