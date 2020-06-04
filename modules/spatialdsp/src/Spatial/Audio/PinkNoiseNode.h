#pragma once

#include <audio/core/audionode.h>

namespace nap
{
    namespace audio
    {
        
        /*
         Copyright (C) 2019 Marcel Smit
         marcel303@gmail.com
         
         Permission is hereby granted, free of charge, to any person
         obtaining a copy of this software and associated documentation
         files (the "Software"), to deal in the Software without
         restriction, including without limitation the rights to use,
         copy, modify, merge, publish, distribute, sublicense, and/or
         sell copies of the Software, and to permit persons to whom the
         Software is furnished to do so, subject to the following
         conditions:
         
         The above copyright notice and this permission notice shall be
         included in all copies or substantial portions of the Software.
         
         THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
         EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
         OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
         NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
         HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
         WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
         FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
         OTHER DEALINGS IN THE SOFTWARE.
         */
        
        // Voss' pink number object to be used for pink noise generation
        
        struct NAPAPI PinkNumber
        {
            int maxKey;
            int key;
            int whiteValues[5];
            int range;
            
            PinkNumber(const int range = 128);
            
            int next();
        };
        

        /**
         * White noise generator
         */
        class NAPAPI PinkNoiseNode : public Node
        {
        public:
            PinkNoiseNode(NodeManager& manager) : Node(manager), mPinkNumber(512) { }
            
            /**
             * Output signal containing the noise
             */
            OutputPin audioOutput = { this };
            
        private:
            void process() override
            {
                auto& buffer = getOutputBuffer(audioOutput);
                for (auto i = 0; i < buffer.size(); ++i)
                    buffer[i] = (mPinkNumber.next() / 256.f) - 1.0f;
            }
            
            PinkNumber mPinkNumber;
            
        };

        
    }
}

