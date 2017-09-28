#pragma once

// Std includes
#include <functional>
#include <set>

// Audio includes
#include <utility/audiotypes.h>
#include <node/audiopin.h>

namespace nap {
    
    namespace audio {
    
        
        // Forward declarations
        class NodeManager;
        
        
        /**
         * An node does audio processing.
         * The node can have an arbitrary number of inputs and outputs, used to connect streams of mono audio between different nodes.
         * Use this as a base class for custom nodes that generate audio output.
         */
        class NAPAPI Node
        {
            friend class NodeManager;
            friend class OutputPin;
            friend class InputPin;
            friend class MultiInputPin;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. The node receives it's buffersize and samplerate from the manager.
             */
            Node(NodeManager& manager);
            ~Node();
            
            /**
             * Returns the internal buffersize of the node system that this node belongs to. 
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
            
            /**
             * Returns all this node's outputs
             */
            const std::set<OutputPin*>& getOutputs() { return mOutputs; }
            
        protected:
            /**
             * Called whenever the sample rate that the node system runs on changes.
             * Can be overwritten to respond to changes
             * @param sampleRate: then new sample rate
             */
            virtual void sampleRateChanged(float sampleRate) { }
            // Called whenever the internal buffer size changes

            /**
             * Called whenever the buffersize that the node system runs on changes.
             * Can be overwriten to respond to changes.
             * @param bufferSize: the new buffersize
             */
            virtual void bufferSizeChanged(int bufferSize) { }
            
            
            /**
             * Use this function within descendants @process() implementation to access the buffers that need to be filled with output.
             * @param: the output that the buffer is requested for
             */
            SampleBuffer& getOutputBuffer(OutputPin& output);
            
            /**
             * @return: The node manager that this node is processed on
             */
            NodeManager& getNodeManager() { return *mNodeManager; }

        private:
            /**
             * Override this method to do the actual audio processing and fill the buffers of this node's outputs with new audio data
             * Use @getOutputBuffer() to access the buffers that have to be filled.
             */
            virtual void process() { }
            
            /*
             * Invoked by OutputPin::pull methods
             * Makes sure all the outputs of this node are being processed up to the current time stamp.
             * Calls process() call if not up to date.
             */
            void update();
            
            /*
             * Used by the node manager to notify the node that the buffer size has changed.
             * @param bufefrSize: the new value
             */
            void setBufferSize(int bufferSize);
            
            /*
             * Used by the node manager to notify the node that the sample rate has changed.
             * @param sampleRate: the new value
             */
            void setSampleRate(float sampleRate) { sampleRateChanged(sampleRate); }
            
            /*
             * Used internally by the node to keep track of all its outputs.
             */
            std::set<OutputPin*> mOutputs;
            
            // The time stamp of the latest calculated sample by this node
            DiscreteTimeValue mLastCalculatedSample = 0;
            
            /**
             * The node manager that this node is processed on
             */
            NodeManager* mNodeManager = nullptr;
            
        };
        
        
        /**
         * Node to provide audio output for the node manager's audio processing, typically sent to an audio interface.
         * The OutputNode is a root node that will be directly processed by the node manager.
         */
        class NAPAPI OutputNode : public Node
        {
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. This node provides audio output for the manager.
             * @param active: true if the node is active and being processed from the moment of creation. This can cause glitches if the node tree and it's parameters are still being build.
             */
            OutputNode(NodeManager& manager, bool active = true);
            
            ~OutputNode();
            
            /**
             * Through this input the node receives buffers of audio samples that will be presented to the node manager as output for its audio processing.
             */
            InputPin audioInput;
            
            /**
             * Set the audio channel that this node's input will be played on by the node manager.
             * @param outputChannel: the channel number
             */
            void setOutputChannel(int outputChannel) { mOutputChannel = outputChannel; }
            
            /**
             * @return: the audio channel that this node's input will be played on by the node manager.
             */
            int getOutputChannel() const { return mOutputChannel; }
            
            /**
             * Sets wether the node will be processed by the audio node manager.
             * On creation the node is inactive by default.
             */
            void setActive(bool active) { mActive = true; }
            
            /**
             * @return: true if the node is currently active and thus being processed (triggered) on every callback.
             */
            bool isActive() const { return mActive; }
            
        private:
            void process() override;
            
            int mOutputChannel = 0;
            
            bool mActive = true;
        };
        
        
        /**
         * This node outputs the audio input that is received from the node system's external input, typically an audio interface.
         * Input from channel @inputChannel can be pulled from @audioOutput plug.
         */
        class NAPAPI InputNode : public Node
        {
            friend class NodeManager;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. This node provides audio output for the manager.
             */
            InputNode(NodeManager& manager) : Node(manager) { }
            
            /**
             * This output will contain the samples received from the node system's external input.
             */
            OutputPin audioOutput = { this };
            
            /**
             * Sets the channel from which this node receives input.
             */
            void setInputChannel(int inputChannel) { mInputChannel = inputChannel; }
            
            /**
             * @return: the channel from which this node receives input.
             */
            int getInputChannel() const { return mInputChannel; }
            
        private:
            void process() override;
            
            int mInputChannel = 0;
        };
        
        
    }
}





