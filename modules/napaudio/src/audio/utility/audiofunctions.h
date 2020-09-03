#pragma once

#include <mathutils.h>

namespace nap
{
    
    namespace audio
    {
        
        /**
         * Wraps value @inc within the range of @bufferSize.
         * @bufferSize is required to be a power of 2!
         */
        inline unsigned int wrap(unsigned int inc, unsigned int bufferSize)
        {
            unsigned int bitMask = bufferSize - 1;
            return inc & bitMask;
        }
        
        
        /**
         * Linear interpolation between v0 and v1. v0 is returned when t = 0 and v1 is returned when t = 1.
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
        
        
        /**
         * Convert a midi notenumber format pitch (floating point for microtonal precision) to a frequency in Herz.
         */
        inline float mtof(float pitch)
        {
            auto res = pitch - 57;
            res /= 12.0;
            res = pow(2.0, res);
            res *= 220.0;
            return res;
        }


        /**
         * Convert amplitude to decibel value.
         */
        inline float toDB(float amplitude)
        {
            return 20 * log10(amplitude);
        }


        /**
         * Convert decibel value to amplitude.
         */
        inline float dbToA(float db, float zero = -48)
        {
            if (db <= zero)
                return 0;

            return powf(10, db / 20.0);
        }

        

        
    }
    
}
