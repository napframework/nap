//
//  EnvelopeFollowerNode.cpp
//  theworks
//
//  Created by Casimir Geelhoed on 07/02/2019.
//
//

#include "EnvelopeFollowerNode.h"


namespace nap {
    
    namespace audio {
        
        void EnvelopeFollowerNode::process(){
            
            auto& outputBuffer = getOutputBuffer(audioOutput);
            auto& inputBuffer = *audioInput.pull();

            // load atomics
            float attackCoef = mAttackCoef;
            float releaseCoef = mReleaseCoef;
            float value = mValue;
            
            for(int i = 0; i < inputBuffer.size(); i++){
                
                float tmp = inputBuffer[i];
                
                if(tmp < 0)
                    tmp = -tmp;
                
                if(tmp > value)
                    value = attackCoef * (value - tmp) + tmp;
                else
                    value = releaseCoef * (value - tmp) + tmp;
                
            }
            
            // store atomic
            mValue = value;
            
            // sound just passes through
            outputBuffer = inputBuffer;
            
        }
        
    }
    
}
