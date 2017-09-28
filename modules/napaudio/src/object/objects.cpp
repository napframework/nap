#include "objects.h"


RTTI_BEGIN_CLASS(nap::audio::Oscillator)
    RTTI_PROPERTY("ChannelCount", &nap::audio::Oscillator::mChannelCount, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Frequency", &nap::audio::Oscillator::mFrequency, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Amplitude", &nap::audio::Oscillator::mAmplitude, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FmInput", &nap::audio::Oscillator::mFmInput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap {
    
    namespace audio {
        
    }
    
}
