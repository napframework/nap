#include "VectorExtension.h"

#include <cmath>

namespace nap
{
    
    float4 tanVec(const float4 value)
    {
        const float * valueElements = (float*)&value;
        
        return float4(
                      tanf(valueElements[0]),
                      tanf(valueElements[1]),
                      tanf(valueElements[2]),
                      tanf(valueElements[3]));
    }
    
    
    float8 tanVec(const float8 value)
    {
        const float * valueElements = (float*)&value;
        
        return float8(
                      tanf(valueElements[0]),
                      tanf(valueElements[1]),
                      tanf(valueElements[2]),
                      tanf(valueElements[3]),
                      tanf(valueElements[4]),
                      tanf(valueElements[5]),
                      tanf(valueElements[6]),
                      tanf(valueElements[7]));
    }

    
    float4 sinVec(const float4 value)
    {
        const float * valueElements = (float*)&value;
        
        return float4(
                      sinf(valueElements[0]),
                      sinf(valueElements[1]),
                      sinf(valueElements[2]),
                      sinf(valueElements[3]));
    }
    
    
    float8 sinVec(const float8 value)
    {
        const float * valueElements = (float*)&value;
        
        return float8(
                      sinf(valueElements[0]),
                      sinf(valueElements[1]),
                      sinf(valueElements[2]),
                      sinf(valueElements[3]),
                      sinf(valueElements[4]),
                      sinf(valueElements[5]),
                      sinf(valueElements[6]),
                      sinf(valueElements[7]));
    }
    
    
    float4 cosVec(const float4 value)
    {
        const float * valueElements = (float*)&value;
        
        return float4(
                      cosf(valueElements[0]),
                      cosf(valueElements[1]),
                      cosf(valueElements[2]),
                      cosf(valueElements[3]));
    }
    
    
    float8 cosVec(const float8 value)
    {
        const float * valueElements = (float*)&value;
        
        return float8(
                      cosf(valueElements[0]),
                      cosf(valueElements[1]),
                      cosf(valueElements[2]),
                      cosf(valueElements[3]),
                      cosf(valueElements[4]),
                      cosf(valueElements[5]),
                      cosf(valueElements[6]),
                      cosf(valueElements[7]));
    }

    
    float4 powVec(const float4 value, const float4 power)
    {
        const float * valueElements = (float*)&value;
        const float * powerElements = (float*)&power;

        return float4(
                      powf(valueElements[0], powerElements[0]),
                      powf(valueElements[1], powerElements[1]),
                      powf(valueElements[2], powerElements[2]),
                      powf(valueElements[3], powerElements[3]));
    }
    
    
    float8 powVec(const float8 value, const float8 power)
    {
        const float * valueElements = (float*)&value;
        const float * powerElements = (float*)&power;

        return float8(
                      powf(valueElements[0], powerElements[0]),
                      powf(valueElements[1], powerElements[1]),
                      powf(valueElements[2], powerElements[2]),
                      powf(valueElements[3], powerElements[3]),
                      powf(valueElements[4], powerElements[4]),
                      powf(valueElements[5], powerElements[5]),
                      powf(valueElements[6], powerElements[6]),
                      powf(valueElements[7], powerElements[7]));
    }
    
}
