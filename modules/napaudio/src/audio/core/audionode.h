#pragma once

// Std includes
#include <functional>
#include <set>

// RTTI includes
#include <rtti/rtti.h>

// Audio includes
#include <audio/utility/audiotypes.h>
#include <audio/core/audiopin.h>
#include <audio/core/process.h>

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
        class NAPAPI Node : public Process
        {
            RTTI_ENABLE(Process)
            
            friend class NodeManager;
            friend class InputPinBase;
            friend class InputPin;
            friend class OutputPin;
            friend class MultiInputPin;
            friend class NodePtrBase;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. The node receives it's buffersize and samplerate from the manager.
             */
            Node(NodeManager& manager);
            virtual ~Node();
            
            /**
             * Returns all this node's outputs
             */
            const std::set<OutputPin*>& getOutputs() const {
                return mOutputs;
            }
            
            /**
             * Returns all this node's inputs
             */
            const std::set<InputPinBase*>& getInputs() const { return mInputs; }
            
            /*
             * Override this method to do the actual audio processing and fill the buffers of this node's outputs with new audio data
             * Use @getOutputBuffer() to access the buffers that have to be filled.
             */
            void process() override { }
            
        protected:
            /**
             * Use this function within descendants @process() implementation to access the buffers that need to be filled with output.
             * @param: the output that the buffer is requested for
             */
            SampleBuffer& getOutputBuffer(OutputPin& output);

            /**
             * @return true if the node is currently registered with a currently existing node manager.
             */
            bool isRegisteredWithNodeManager() const { return mRegisteredWithNodeManager.load(); }
            
        private:
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
            
            std::set<OutputPin*> mOutputs; // Used internally by the node to keep track of all its outputs.
            std::set<InputPinBase*> mInputs; // Used internally by the node to keep track of all its inputs.

            // Set to true when the node has made itself known with the node manager. This registration is deferred to the audio thread so it has to be tracked by this boolean.
            std::atomic<bool> mRegisteredWithNodeManager = { false };
        };
        
        

    }
}





