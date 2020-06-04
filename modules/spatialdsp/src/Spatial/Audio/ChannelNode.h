
#pragma once

#include <audio/core/audionode.h>
#include <audio/core/audiopin.h>


namespace nap
{
    namespace audio
    {
        
        /**
         * ChannelNode is a @Node that pulls an input buffer in its process function.
         * This input buffer is exposed to an external system (that can run in a different root process).
         * Its output buffer is set by an external system.
         */
        class NAPAPI ChannelNode : public Node
        {
            
        public:
            ChannelNode(NodeManager& nodeManager);
            
            InputPin audioInput = { this };
            OutputPin audioOutput = { this };

            
            /**
             * Sets the output to zero.
             */
            void setOutputZero(bool outputZero);
            
            /**
             * Returns the last input buffer.
             */
            SampleBuffer* getLastInputBuffer();
            
            /**
             * Sets the output buffer that will be outputted when this node is pulled.
             */
            void setOutputBuffer(SampleBuffer* buffer);
            
            /**
             * Pulls the input buffer, and saves a pointer to it.
             */
            void process() override;
            
        private:
            bool mOutputZero = false; // if set to true, this node will output a zero buffer instead of the received buffer.
            
            // Buffer filled with zeroes.
            // Note: if the buffer size will be able to change dynamically, this zero buffer should resize with it.
            // It would be better if there was one 'zero buffer' available in the audio service, that deals with this automatically.
            audio::SampleBuffer mZeroBuffer;
            
            // Pointer to last input buffer.
            audio::SampleBuffer* mLastInputBuffer = &mZeroBuffer;
            
        };
        
        
        /**
         * Virtual base class. A ChannelsSetterProcess is a process that calculates the output buffers of multiple ChannelNodes based on its inputs.
         * The processing function needs to be overriden by subclasses. It should set the output buffer based on the input buffers.
         * Input buffers can be read by calling getInputBuffer() and output buffers can be set by calling setOutputBuffer().
         */
        class NAPAPI ChannelsSetterProcess : public Process
        {
        public:
            ChannelsSetterProcess(NodeManager& nodeManager, std::vector<SafePtr<ChannelNode>> channels) : Process(nodeManager), mChannels(channels) { }
            
        protected:
            /**
             * Returns the input buffer retrieved from the channel 'index'.
             */
            SampleBuffer* getInputBuffer(int channel){ return mChannels[channel]->getLastInputBuffer(); }
            
            /**
             * Sets the output buffer for channel 'channel'. Should be called one time for each channel in calculateOuputBuffers().
             */
            void setOutputBuffer(int channel, SampleBuffer* buffer){
                mChannels[channel]->setOutputBuffer(buffer);
            }
            
            int getChannelCount() { return mChannels.size(); }
            
        private:
            std::vector<SafePtr<ChannelNode>> mChannels;
            
        };
        
    }
}
