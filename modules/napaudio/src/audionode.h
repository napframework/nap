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
        
        
        /**
         * An audio input is used by audio node to connect it to other nodes.
         * The input connects one channel (mono) audio.
         */
        class NAPAPI AudioInput {
        public:
            AudioInput() = default;
            
            /**
             * Destructor. If the input is connected on destruction the connection will be broken first.
             */
            ~AudioInput();
            
            /**
             * This method can be used by the node to pull one sample buffer output from the connected audio output.
             * @return If the AudioInput is not connected or somewhere down the graph silence is being output nullptr can be returned.
             */
            SampleBufferPtr pull();
            
            /**
             * Connects another node's @AudioOutput to this input.
             * If either this ipnut or the connected output is already connected it will be disconnected first.
             * @param connection: The output that this @AudioInput will be connected to.
             */
            void connect(AudioOutput& connection);
            
            
            /**
             * Disconnects this input from the connected output
             */
            void disconnect();
            
            /**
             * Checks wether the input is connected
             */
            bool isConnected() const { return mConnection; }
            
        private:
            /*
             * The audio output connected to this input.
             * When it is a nullptr this input is not connected.
             */
            AudioOutput* mConnection = nullptr;
        };
        
        
        /**
         * An audio output is used by audio node to connect it to other nodes.
         * The output connects one channel (mono) audio.
         * It outputs a pointer to an owned @SampleBuffer.
         * The PullFunction of this class calls a calculate function on the node it belongs to.
         */
        class NAPAPI AudioOutput {
            friend class AudioNode;
            friend class AudioInput;
            
        public:
            /**
             * Constructor takes node and a member function of the node that will be called to calculate the content of the next buffer.
             * @param parent: the owner node if this output
             * @param calcFunctionPtr: a pointer to member function to a member function of the node that calculates this output's output samples
             */
            template <typename U>
            AudioOutput(U* parent, void(U::*calcFunctionPtr)(SampleBuffer&));
            
            ~AudioOutput();            
            
            /**
             * Disconnects the output from the connected input.
             */
            void disconnect();
            
            /**
             * Checks wether the output is connected
             */
            bool isConnected() const { return mConnection; }
            
        protected:
            /**
             * The buffer containing the latest output of the plug
             */
            SampleBuffer mBuffer;
                        
        private:
            using CalculateFunction = std::function<void(SampleBuffer&)>;
            
            // Used by @AudioInput to poll this output for a new buffer of output samples
            SampleBufferPtr pull();
            
            // Used by the @AudioNodeManager to resize the internal buffers when necessary
            void setBufferSize(int bufferSize);

            // The time stamp of the latest calculated sample in the buffer
            DiscreteTimeValue mLastCalculatedSample = 0;
            
            // The functor that will be used to compute the contents of the next buffer
            CalculateFunction mCalculateFunction = nullptr;
            
            // The node that owns this output
            AudioNode* mNode = nullptr;
            
            // The input that this output is connected to, nullptr when disconnected
            // This pointer is kept so the connection can be broken on destruction.
            AudioInput* mConnection = nullptr;
        };
        
        
        /**
         * An node does audio processing.
         * The node can have an arbitrary number of inputs and outputs, used to connect streams of mono audio between different nodes.
         * Use this as a base class for custom nodes that generate audio output.
         */
        class NAPAPI AudioNode {
            friend class AudioNodeManager;
            friend class AudioOutput;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. The node receives it's buffersize and samplerate from the manager.
             */
            AudioNode(AudioNodeManager& manager);
            ~AudioNode();
            
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
             * The node manager that this node is processed on
             */
            AudioNodeManager* mAudioNodeManager = nullptr;
            
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
            
            /*
             * Used internally by the node to keep track of all its outputs.
             */
            std::set<AudioOutput*> mOutputs;
        };
        
        
        /**
         * This is a special case audio node that receives a call from the node manager every time a new output buffer has to be processed for the audio callback.
         * Typically this call will be used to process the nodes' inputs and provide the incoming sample buffers to node manager as output for the audio interface or record it into a buffer.
         */
        class NAPAPI AudioTrigger : public AudioNode {
            friend class AudioNodeManager;
            
        public:
            /**
             * @param nodeManager: the node manager that this node will be registered to and processed by. The node receives it's buffersize and samplerate from the manager. It will call this node's @trigger method every time a new buffer of audio has to be calculated as output.
             */
            AudioTrigger(AudioNodeManager& nodeManager);
            ~AudioTrigger();
            
        private:
            /**
             * Override this method to add behaviour every time a new buffer is processed by the node system.
             */
            virtual void trigger() = 0;
        };
        
        
        /**
         * Node to provide audio output for the node manager's audio processing, typically sent to an audio interface.
         */
        class NAPAPI AudioOutputNode : public AudioTrigger {
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. This node provides audio output for the manager.
             */
            AudioOutputNode(AudioNodeManager& manager) : AudioTrigger(manager) { }
            
            /**
             * Through this input the node receives buffers of audio samples that will be presented to the node manager as output for its audio processing.
             */
            AudioInput audioInput;
            
            /**
             * Set the audio channel that this node's input will be played on by the node manager.
             * @param outputChannel: the channel number
             */
            void setOutputChannel(int outputChannel) { mOutputChannel = outputChannel; }
            
            /**
             * @return: the audio channel that this node's input will be played on by the node manager.
             */
            int getOutputChannel() const { return mOutputChannel; }
            
        private:
            void trigger() override final;
            
            int mOutputChannel = 0;
        };
        
        
        /**
         * This node outputs the audio input that is received from the node system's external input, typically an audio interface.
         * Input from channel @inputChannel can be pulled from @audioOutput plug.
         */
        class NAPAPI AudioInputNode : public AudioNode {
            friend class AudioNodeManager;
            
        public:
            /**
             * @param manager: the node manager that this node will be registered to and processed by. This node provides audio output for the manager.
             */
            AudioInputNode(AudioNodeManager& manager) : AudioNode(manager) { }
            
            /**
             * This output will contain the samples received from the node system's external input.
             */
            AudioOutput audioOutput = { this, &AudioInputNode::fill };
            
            /**
             * Sets the channel from which this node receives input.
             */
            void setInputChannel(int inputChannel) { mInputChannel = inputChannel; }
            
            /**
             * @return: the channel from which this node receives input.
             */
            int getInputChannel() const { return mInputChannel; }
            
        private:
            void setBufferSize(int bufferSize);
            void fill(SampleBuffer&);
            
            int mInputChannel = 0;
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





