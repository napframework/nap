
#include "PinkNoiseNode.h"

#include <stdlib.h>


namespace nap
{
    namespace audio
    {
        
        PinkNumber::PinkNumber(const int _range)
        {
            maxKey = 0x1f; // five bits set
            
            range = _range;
            key = 0;
            
            for (int i = 0; i < 5; ++i)
            {
                whiteValues[i] = rand() % (range/5);
            }
        }

        int PinkNumber::next()
        {
            const int lastKey = key;
            
            key++;
            
            if (key > maxKey)
                key = 0;
            
            // exclusive-or previous value with current value. this gives a list of bits that have changed
            
            int diff = lastKey ^ key;
            
            int sum = 0;
            
            for (int i = 0; i < 5; ++i)
            {
                // if bit changed get new random number for corresponding whiteValue
                
                if (diff & (1 << i))
                {
                    whiteValues[i] = rand() % (range/5);
                }
                
                sum += whiteValues[i];
            }
            
            return sum;
        }
                
    }
}

