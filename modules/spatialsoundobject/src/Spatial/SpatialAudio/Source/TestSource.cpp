#include "TestSource.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::TestSource)
RTTI_END_CLASS


namespace nap
{

    namespace spatial
    {

        bool TestSource::init(utility::ErrorState& errorState)
        {
            // initialize wavetable
            mWaveTable = getSpatialService().getAudioService().getNodeManager().makeSafe<audio::WaveTable>(65536);

            // initialize oscillator
            mOscillator = std::make_unique<audio::NodeObjectInstance<audio::OscillatorNode>>();
            if (!mOscillator->init(getSpatialService().getAudioService().getNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize TestSource");
                return false;
            }
            mOscillator->get()->setWave(mWaveTable);
            mOscillator->get()->setFrequency(300);
            mOscillator->get()->setAmplitude(1.);

            // intialize routing
            redistribute();

            return true;
        }

        audio::OutputPin* TestSource::getInputChannel(int index)
        {
            return &mOscillator->get()->output;
        }

    }

}