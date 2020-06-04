#pragma once 

// Spatial Audio includes
#include <Spatial/Audio/FastGainNode.h>
#include <Spatial/Audio/PinkNoiseNode.h>

// Audio includes
#include <audio/utility/safeptr.h>


namespace nap
{
    namespace audio
    {
        class AudioService;
    }
    
    namespace spatial
    {

        /** 
         * Class to generate mono white noise with an adjustable level control. Can be connected to specific channels by the @MultiSpeakerSetup as a means to test speakers.
         */
        class NAPAPI SpeakerTest {
            
        public:
            SpeakerTest(audio::AudioService& audioService);
            
            /**
             * Returns the output signal.
             */
            audio::OutputPin& getOutput();
            
            
            /**
             * Sets the level of the noise signal.
             */
            void setLevel(float level);
            
        private:
            audio::SafeOwner<audio::FastGainNode> mGainNode;
            audio::SafeOwner<audio::PinkNoiseNode> mNoiseNode;
            
        };
        
    }
}
