#pragma once

// Nap includes
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audiopin.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Interface for any class that exposes multichannel audio output.
         */
        class NAPAPI IMultiChannelOutput
        {
        public:
			/**
			 * Destructor
			 */
        	virtual ~IMultiChannelOutput() = default;
			
            /**
             * Override this method to specify the number of audio channels output by this object.
             */
            virtual int getChannelCount() const = 0;
            
            /**
             * Override this to return the output pin that outputs audio data for the specified channel.
             */
            virtual OutputPin* getOutputForChannel(int channel) = 0;
            
            /**
             * This function returns an output pin for a given channel, but returns nullptr if the channel is out of bounds.
             */
            OutputPin* tryGetOutputForChannel(unsigned int);
            
        };
        
        
        /**
         * Interface for any class that exposes multichannel audio input.
         */
        class NAPAPI IMultiChannelInput
        {
        public:
			/**
			 * Destructor
			 */
        	virtual ~IMultiChannelInput() = default;
			
            /**
             * This method has to be overwritten to connect an output pin from another object to this object's input for the given @channel.
             */
            virtual void connect(unsigned int channel, OutputPin& pin) { }
            
            /**
             * This method has to be overwritten to return the number of input channels of audio this object receives.
             */
            virtual int getInputChannelCount() const { return 0; }
            
            /**
             * This method calls @connect() but first checks wether @channel is not out of bounds.
             */
            void tryConnect(unsigned int channel, OutputPin& pin);
            
            /**
             * Convenience method that connects the outputs of @inputObject to the inputs of this object.
             * If this object has more input channels than inputObject has output channels they will be repeated.
             */
            void connect(IMultiChannelOutput& inputObject);
        };

        
    }
    
}
