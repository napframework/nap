namespace nap
{
    
    namespace audio
    {
        
        static unsigned int wrap(unsigned int inc, unsigned int bufferSize)
        {
            unsigned int bitMask = bufferSize - 1;
            return inc & bitMask;
        }
        
        
        // linear interpolation between v0 and v1. v0 is returned when t = 0 and v1 is returned when t = 1.
        template <typename T>
        inline T lerp(const T& v0, const T& v1, const T& t)
        {
            return v0 + t * (v1 - v0);
        }                                
        
    }
    
}
