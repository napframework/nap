#pragma once

// Std includes
#include <functional>
#include <set>

// RTTI includes
#include <rtti/rtti.h>

// Audio includes
#include <audio/utility/audiotypes.h>
#include <audio/core/audiopin.h>

namespace nap
{
    
    namespace audio
    {
    
        
        // Forward declarations
        class NodeManager;
        
        
        /**
         * A node performs audio processing and is the smallest unit of a DSP network.
         * The node can have an arbitrary number of inputs and outputs, used to connect streams of mono audio between different nodes.
         * Use this as a base class for custom nodes that generate audio output.
         */
        class NAPAPI Node
        {
            RTTI_ENABLE()
            
            friend class NodeManager;
            friend class OutputPin;
            friend class InputPin;
            friend class MultiInputPin;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. The node receives it's buffersize and samplerate from the manager.
             */
            Node(NodeManager& manager);
            virtual ~Node();
            
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
        
        

    }
}





