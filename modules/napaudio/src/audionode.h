#pragma once

// Std includes
#include <functional>
#include <set>

// Audio includes
#include "audiotypes.h"

namespace nap {
    
    
    namespace audio {
    
        
        // Forward declarations
        class AudioNodeManager;
        class AudioNode;
        class AudioOutput;
        
        /*
         * An audio input is used by audio node to connect other audio nodes as inputs
         */
        class AudioInput {
        public:
            AudioInput() = default;
            SampleBufferPtr pull();
            void connect(AudioOutput& connection) { mConnection = &connection; }
            
        private:
            AudioOutput* mConnection = nullptr;
        };
        
        
        /* 
         * An audio input is used by audio node to connect to other audio nodes and to poll output samples
         * It outputs a pointer to a SampleBuffer that it manages itself
         * The PullFunction of the output calls a calculate function on the operator to calculate the new buffer
         */
        class AudioOutput {
            friend class AudioNode;
            friend class AudioInput;
            
        public:
            AudioOutput(AudioNode* node);
            ~AudioOutput();
            
            /*
             * Constructor takes node and a member function of the node
             * that will be called to calculate the content of the next buffer
             */
            template <typename U>
            AudioOutput(U* parent, void(U::*calcFunctionPtr)(SampleBuffer&));
            
            SampleBufferPtr pull();
            
        protected:
            // The buffer containing the latest output of the plug
            SampleBuffer mBuffer;
                        
        private:
            using CalculateFunction = std::function<void(SampleBuffer&)>;
            
            // Used by the AudioNodeManager to resize the internal buffers when necessary
            void setBufferSize(int bufferSize);

            // The time stamp of the latest calculated sample in the buffer
            DiscreteTimeValue mLastCalculatedSample = 0;
            
            // The functor that will be used to compute the contents of the next buffer
            CalculateFunction mCalculateFunction = nullptr;
            
            // The node that contains this output
            AudioNode* mNode = nullptr;
        };
        
        
        /*
         * An node does audio processing.
         * Use this as a base class for operators that generate audio output.
         */
        class AudioNode {
            friend class AudioNodeManager;
            friend class AudioOutput;
            
        public:
            AudioNode(AudioNodeManager& manager);
            ~AudioNode();
            
            int getBufferSize() const;
            float getSampleRate() const;
            DiscreteTimeValue getSampleTime() const;
            
        protected:
            // Called by the audio service when the sample rate changes
            virtual void sampleRateChanged(float sampleRate) { }
            // Called whenever the internal buffer size changes
            virtual void bufferSizeChanged(int bufferSize) { }
            
            AudioNodeManager* mAudioNodeManager = nullptr;
            
        private:
            // Used by the AudioNodeManager to resize the internal buffers when necessary
            void setBufferSize(int bufferSize);
            void setSampleRate(float sampleRate) { sampleRateChanged(sampleRate); }
            
            std::set<AudioOutput*> mOutputs;
        };
        
        
        class AudioTrigger : public AudioNode {
            friend class AudioNodeManager;
            
        public:
            AudioTrigger(AudioNodeManager& service);
            ~AudioTrigger();
            
        private:
            virtual void trigger() = 0;
        };
        
        
        /*
         * Node to generate audio output for the audio service's device outputs.
         * Use @triggerInput to pull a new buffer into @audioInput for hardware output channel @outputChannel
         */
        class AudioOutputNode : public AudioTrigger {
        public:
            AudioOutputNode(AudioNodeManager& manager) : AudioTrigger(manager) { }
            
            AudioInput audioInput;
            int outputChannel = 0;
            
        private:
            void trigger() override final;
        };
        
        
        /*
         * Node to provide audio input from the audio service's device inputs.
         * Input from channel @inputChannel can be pulled from @audioOutput plug.
         */
        class AudioInputNode : public AudioNode {
            friend class AudioNodeManager;
            
        public:
            AudioInputNode(AudioNodeManager& service) : AudioNode(service) { }
            
            AudioOutput audioOutput = { this, &AudioInputNode::fill };
            
            int inputChannel = 0;
            
        private:
            void setBufferSize(int bufferSize);
            void fill(SampleBuffer&);
        };
        
        
        // --- TEMPLATE DEFINITIONS --- //
        
        template <typename U>
        AudioOutput::AudioOutput(U* node, void(U::*calcFunctionPtr)(SampleBuffer&))
        {
            node->mOutputs.emplace(this);
            mNode = node;
            mCalculateFunction = std::bind(calcFunctionPtr, node, std::placeholders::_1);
            setBufferSize(mNode->getBufferSize());
        }
        
    }
}





