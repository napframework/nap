#include "oscillator.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::WaveTableResource)
    RTTI_PROPERTY("Size", &nap::audio::WaveTableResource::mSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::Oscillator)
    RTTI_PROPERTY("Frequency", &nap::audio::Oscillator::mFrequency, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Amplitude", &nap::audio::Oscillator::mAmplitude, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("FmInput", &nap::audio::Oscillator::mFmInput, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("WaveTable", &nap::audio::Oscillator::mWaveTable, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::ParallelNodeObjectInstance<nap::audio::OscillatorNode>)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {

        bool WaveTableResource::init(utility::ErrorState& errorState)
        {
            mWave = mNodeManager.makeSafe<WaveTable>(mSize);
            return true;
        }

    }
    
}
