#pragma once

#include <audio/core/graph.h>

namespace nap
{

    namespace audio
    {

        inline float toDB(ControllerValue amplitude)
        {
            return 20 * log10(amplitude);
        }

        
        inline ControllerValue dbToA(float db, float zero = -48)
        {
            if (db <= zero)
                return 0;

            return powf(10, db / 20.0);
        }

    }

}
