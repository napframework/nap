#include <cmath>

namespace nap
{   
    namespace audio
    {
        
        /**
         * Wraps value given by inc within the range of bufferSize.
		 * @param inc incremental wrap value
         * @param bufferSize size of the buffer, required to be a power of 2!
         */
        inline unsigned int wrap(unsigned int inc, unsigned int bufferSize)
        {
            unsigned int bitMask = bufferSize - 1;
            return inc & bitMask;
        }
        
        
        /**
         * Linear interpolation between v0 and v1. v0 is returned when t = 0 and v1 is returned when t = 1.
		 * TODO: DEPRECATE, use default math::lerp<T>, which is optimized for SSL instruction set.
		 * @param v0 min value
		 * @param v1 max value
		 * @param t lerp value (0-1)
         */
        template <typename T>
        inline T lerp(const T& v0, const T& v1, const T& t)
        {
            return v0 + t * (v1 - v0);
        }                                
        

        /**
         * Stereo equal power panning function.
         * @param panning: value between 0 and 1.0, 0 meaning far left, 0.5 center and 1.0 far right.
         * @param left: left channel gain will be stored in this variable
         * @param right: right channel gain will be stored in this variable
         */
        template <typename T>
        inline void equalPowerPan(const T& panning, T& left, T& right)
        {
            left = cos(panning * 0.5 * M_PI);
            right = sin(panning * 0.5 * M_PI);
        }        
        
    }
    
}
