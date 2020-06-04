
#include "SpeakerTest.h"

// Audio includes
#include <audio/service/audioservice.h>

// Utility include (for dbToA)
#include <Spatial/Utility/AudioFunctions.h>

namespace nap
{
    namespace spatial
    {
        
        SpeakerTest::SpeakerTest(audio::AudioService& audioService) {
            auto& nodeManager = audioService.getNodeManager();
            mNoiseNode = nodeManager.makeSafe<audio::PinkNoiseNode>(nodeManager);
            mGainNode = nodeManager.makeSafe<audio::FastGainNode>(nodeManager);
            mGainNode->audioInput.enqueueConnect(mNoiseNode->audioOutput);
            
            // set default level
            mGainNode->setGain(audio::dbToA(-6.));
            
        }
        
        /**
         * Returns the output signal.
         */
        audio::OutputPin& SpeakerTest::getOutput() {
            return mGainNode->audioOutput;
        }
        
        /**
         * Sets the level of the noise signal.
         */
        void SpeakerTest::setLevel(float level){
            if(level >= 0. && level <= 1.)
                mGainNode->setGain(level);
        }
        
    }
}
