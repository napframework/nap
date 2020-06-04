#pragma once

// Audio includes
#include <audio/utility/linearsmoothedvalue.h>

#include <Spatial/Utility/VectorExtension.h>

namespace nap
{
    namespace audio
    {
        
        /**
         * Helper object to calculate a multiple of 4 or 8 biquad filters simultaneously with SSE or AVX vector extensions using @float4 or @float8.
         * @real should be @float4 or @float8.
         */
        template <typename real>
        class NAPAPI BiquadFilter
        {
        public:
            BiquadFilter() : a0(real(0), 64), a1(real(0), 64), a2(real(0), 64), b1(real(0), 64), b2(real(0), 64), gain(real(0), 64), h1(0), h2(0) { }

            BiquadFilter(const BiquadFilter&) = delete;
            BiquadFilter& operator=(const BiquadFilter&) = delete;

            /**
             * Sets the coefficient of all the filters.
             */
            void setCoefficients(real _a0, real _a1, real _a2, real _b1, real _b2, real _gain)
            {
                a0.setValue(_a0);
                a1.setValue(_a1);
                a2.setValue(_a2);
                b1.setValue(_b1);
                b2.setValue(_b2);
                gain.setValue(_gain);
            }
            
            /**
             * Process one input sample for all the filters simultaneously.
             */
            real process(const real value)
            {
                const real result = value * a0.getNextValue() + h1;
                
                h1 = value * a1.getNextValue() + h2 - b1.getNextValue() * result;
                h2 = value * a2.getNextValue()      - b2.getNextValue() * result;
                
                return result * gain.getNextValue();
            }

        private:
            LinearSmoothedValue<real> a0;
            LinearSmoothedValue<real> a1;
            LinearSmoothedValue<real> a2;
            LinearSmoothedValue<real> b1;
            LinearSmoothedValue<real> b2;
            LinearSmoothedValue<real> gain;
            real h1;
            real h2;
        };
        
    }
}
