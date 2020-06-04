#include "Granulator.h"

#include <audio/core/nestednodemanager.h>
#include <audio/core/voice.h>
#include <audio/object/circularbuffer.h>
#include <audio/object/bufferplayer.h>
#include <audio/core/polyphonic.h>

namespace nap
{

    namespace audio
    {

        bool GranulatorInstance::Channel::init(int channelIndex, int internalBufferSize, int voiceCount, Voice& voice, SafePtr<CircularBufferNode> circularBuffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mCircularBuffer = circularBuffer;
            return init(channelIndex, internalBufferSize, voiceCount, voice, pitchTranslator, nodeManager, errorState);
        }


        bool GranulatorInstance::Channel::init(int channelIndex, int internalBufferSize, int voiceCount, Voice& voice, SafePtr<MultiSampleBuffer> buffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mBuffer = buffer;
            return init(channelIndex, internalBufferSize, voiceCount, voice, pitchTranslator, nodeManager, errorState);
        }


        void GranulatorInstance::Channel::updateAudio(DiscreteTimeValue sampleTime)
        {

            if (mActive)
            {
                // Calculate deltaTime in miliseconds (because mDuration is also in miliseconds).
                DiscreteTimeValue deltaTimeInSamples = sampleTime - mLastSampleTime;
                TimeValue deltaTime = (deltaTimeInSamples / mNestedNodeManager->getNestedNodeManager().getSampleRate()) * 1000.f;

                // Calculate the deltaPhase (the increase in phase).
                // A higher density increases the speed of the phase
                // A higher duration (= grain size) decreases the speed of the phase
                float deltaPhase = deltaTime * mDensity / mDuration;


                // Increase the phase by deltaPhase.
                mCurrentPhase += deltaPhase;


                // If we are 'about to spawn', spawn a grain if the phase has reached mIrregularityOffset.
                if(mAboutToSpawn && mCurrentPhase > mIrregularityOffset)
                {
                    makeGrain();
                    mAboutToSpawn = false;
                }


                // If the phase reached 1..
                if(mCurrentPhase > 1.f)
                {
                    // we decrease the phase by 1..
                    mCurrentPhase -= 1.f;

                    // .. and we will spawn a grain as soon as the phase reaches the 'irregularity offset'.
                    // (the 'irregularity offset' is a value between 0 and the value of 'irregularity' [0-1]).
                    mAboutToSpawn = true;
                    mIrregularityOffset = mIrregularity * math::random(0.f, 1.f);
                }

            }

            mLastSampleTime = sampleTime;
        }


        void GranulatorInstance::Channel::makeGrain()
        {
            auto voice = mPolyphonic->findFreeVoice();
            if (voice == nullptr)
                return;
            auto gain = voice->getObject<ParallelNodeObjectInstance<GainNode>>("gain")->getChannel(0);
            auto envelope = voice->getObject<EnvelopeInstance>("envelope");

            auto transpose = mTranspose + (-1 + 2 * math::random(0.f, 1.f)) * mTransposeDeviation;
            auto pitch = mPitchTranslator->translate((transpose + 12) / 24.f);

            switch (mShape)
            {
                case Shape::hanning:
                {
                    envelope->setSegmentData(0, mDuration * 0.5, 1.0, false, false, true);
                    envelope->setSegmentData(1, mDuration * 0.5, 0.0, false, false, true);
                    break;
                }
                case Shape::expodec:
                {
                    float limitedAttackDecay = 0.f;
                    if (mAttackDecay > mDuration * 0.5)
                        limitedAttackDecay = mDuration * 0.5;
                    else {
                        limitedAttackDecay = mAttackDecay;
                        envelope->setSegmentData(0, limitedAttackDecay, 1.0, false, false, false);
                        envelope->setSegmentData(1, mDuration - limitedAttackDecay, 0.0, false, true, false);
                    }
                    break;
                }
                case Shape::rexpodec:
                {
                    float limitedAttackDecay = 0.f;
                    if (mAttackDecay > mDuration * 0.5)
                        limitedAttackDecay = mDuration * 0.5;
                    else {
                        limitedAttackDecay = mAttackDecay;
                        envelope->setSegmentData(0, mDuration - limitedAttackDecay, 1.0, false, true, false);
                        envelope->setSegmentData(1, limitedAttackDecay, 0.0, false, false, false);
                    }
                    break;
                }
            }

            auto amplitude = mAmplitude * mAmplitudeScaling;
            auto amplitudeNoisy = amplitude * (1 + (math::random(0.f, 1.f) * mAmplitudeDev * 2) - mAmplitudeDev);
            gain->setGain(amplitudeNoisy, 0);
            gain->inputs.disconnectAll();
            gain->inputs.connect(*envelope->getOutputForChannel(0));

            if (mCircularBuffer != nullptr)
            {
                auto samplesPerMillisecond = mCircularBuffer->getSampleRate() / 1000.0;
                DiscreteTimeValue pos = 0;
                if (pitch > 1)
                    pos = int((mDuration * (pitch - 1) + mDiffusion * math::random(0.f, 1.f)) * samplesPerMillisecond) + mCircularBuffer->getBufferSize() * 2;
                else
                    pos = int(mDiffusion * math::random(0.f, 1.f) * samplesPerMillisecond) + mCircularBuffer->getBufferSize() * 2;
                // fixme: why does it need bufferSize * 2?

                auto circularBufferPlayer = voice->getObject<ParallelNodeObjectInstance<CircularBufferPlayerNode>>("circularBufferPlayer")->getChannel(0);
                circularBufferPlayer->play(*mCircularBuffer, pos, pitch);
                gain->inputs.connect(circularBufferPlayer->audioOutput);
            }

            else if (mBuffer != nullptr)
            {
                auto samplesPerMillisecond = 1.f; // TODO this is not right, has to be adjusted to the buffer's samplerate
                DiscreteTimeValue pos = int((mPosition + mDiffusion * math::random(0.f, 1.f)) * samplesPerMillisecond);

                auto bufferPlayer = voice->getObject<ParallelNodeObjectInstance<BufferPlayerNode>>("bufferPlayer")->getChannel(0);
                bufferPlayer->stop();
                bufferPlayer->setBuffer(mBuffer);
                bufferPlayer->play(0, pos, pitch);
                gain->inputs.connect(bufferPlayer->audioOutput);
            }

            mPolyphonic->play(voice, 0);
        }


        bool GranulatorInstance::Channel::init(int channelIndex, int internalBufferSize, int voiceCount, Voice& voice, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            mChannelIndex = channelIndex;
            mPitchTranslator = pitchTranslator;

            mNestedNodeManager = std::make_unique<NestedNodeManagerInstance>();
            if (!mNestedNodeManager->init(nodeManager, 0, 1, internalBufferSize, errorState))
            {
                errorState.fail("Failed to initialize nested node manager.");
                return false;
            }

            mPolyphonic = std::make_unique<PolyphonicInstance>();
            if (!mPolyphonic->init(voice, voiceCount, true, 1, mNestedNodeManager->getNestedNodeManager(), errorState))
            {
                errorState.fail("Failed to initialize Polyphonic");
                return false;
            }

            mOutput = std::make_unique<NodeObjectInstance<OutputNode>>();
            if (!mOutput->init(mNestedNodeManager->getNestedNodeManager(), errorState))
            {
                errorState.fail("Failed to create nested nodemanager's output");
                return false;
            }
            mOutput->get()->setOutputChannel(0);

            mOutput->AudioObjectInstance::connect(*mPolyphonic);
            mNestedNodeManager->getNestedNodeManager().registerRootProcess(*mOutput->get());

            mNestedNodeManager->getNestedNodeManager().mUpdateSignal.connect([&](DiscreteTimeValue sampleTime){ updateAudio(sampleTime); });


            // Initial phase.
            mCurrentPhase = mChannelIndex / float(mChannelCount);

            return true;
        }



        void GranulatorInstance::Channel::setActive(bool active)
        {
            if (active == mActive)
                return;

            mActive = active;
        }


        void GranulatorInstance::Channel::setTranspose(float value)
        {
            mTranspose = value;
        }


        void GranulatorInstance::Channel::setDuration(TimeValue value)
        {
            mDuration = value;
        }


        void GranulatorInstance::Channel::setDensity(float value)
        {
            mDensity = value;
        }


        void GranulatorInstance::Channel::setPosition(double time)
        {
            mPosition = time;
        }


        OutputPin* GranulatorInstance::Channel::getOutput()
        {
            return mNestedNodeManager->getOutputForChannel(0);
        }


        void GranulatorInstance::Channel::setSource(SafePtr<CircularBufferNode> buffer)
        {
            mCircularBuffer = buffer;
            mBuffer = nullptr;
        }


        void GranulatorInstance::Channel::setSource(SafePtr<MultiSampleBuffer> buffer)
        {
            mBuffer = buffer;
            mCircularBuffer = nullptr;
        }


        bool GranulatorInstance::initHelper(int channelCount, int internalBufferSize, int voiceCount, CircularBufferInstance* circularBuffer, SafePtr<MultiSampleBuffer> buffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto mEqualPowerTable = std::make_unique<EqualPowerTable>(nodeManager);
            mEqualPowerTable->mID = "equalPowerTable";
            mEqualPowerTable->mSize = 128;
            if (!mEqualPowerTable->init(errorState))
            {
                errorState.fail("Failed to initialize equal power table");
                return false;
            }

            Envelope envelope;
            envelope.mID = "envelope";

            EnvelopeNode::Segment attack;
            attack.mDuration = 5.f;
            attack.mDestination = 1.f;
            attack.mDurationRelative = false;
            attack.mMode = RampMode::Linear;
            attack.mTranslate = false;

            EnvelopeNode::Segment decay;
            decay.mDuration = 200.f;
            decay.mDestination = 0.f;
            decay.mDurationRelative = false;
            decay.mMode = RampMode::Linear;
            decay.mTranslate = false;

            envelope.mSegments.emplace_back(attack);
            envelope.mSegments.emplace_back(decay);

            envelope.mEqualPowerTable = mEqualPowerTable.get();

            CircularBufferPlayer circularBufferPlayer;
            circularBufferPlayer.mID = "circularBufferPlayer";
            circularBufferPlayer.mChannelCount = 1;

            BufferPlayer bufferPlayer;
            bufferPlayer.mID = "bufferPlayer";
            bufferPlayer.mChannelCount = 1;
            bufferPlayer.mAutoPlay = false;

            Gain gain;
            gain.mID = "gain";
            gain.mChannelCount = 1;
            gain.mInputs.emplace_back(&envelope);
            // One of the buffer players will be connected to the gain when the voice is started

            Voice voice;
            voice.mObjects.emplace_back(&envelope);
            voice.mObjects.emplace_back(&circularBufferPlayer);
            voice.mObjects.emplace_back(&bufferPlayer);
            voice.mObjects.emplace_back(&gain);
            voice.mOutput = &gain;
            voice.mEnvelope = &envelope;

            for (auto channel = 0; channel < channelCount; ++channel)
            {
                auto granChannel = nodeManager.makeSafe<Channel>();
                bool success = false;
                if (circularBuffer != nullptr)
                    success = granChannel->init(channel, internalBufferSize, voiceCount, voice, circularBuffer->getChannel(0), pitchTranslator, nodeManager, errorState);
                else
                    success = granChannel->init(channel, internalBufferSize, voiceCount, voice, buffer, pitchTranslator, nodeManager, errorState);
                if (!success)
                {
                    errorState.fail("Failed to init granulator channel.");
                    return false;
                }
                mChannels.emplace_back(std::move(granChannel));
            }

            return true;
        }


        bool GranulatorInstance::init(int channelCount, int internalBufferSize, int voiceCount, CircularBufferInstance& circularBuffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            return initHelper(channelCount, internalBufferSize, voiceCount, &circularBuffer, nullptr, pitchTranslator, nodeManager, errorState);
        }


        bool GranulatorInstance::init(int channelCount, int internalBufferSize, int voiceCount, SafePtr<MultiSampleBuffer> buffer, SafePtr<TableTranslator<float>> pitchTranslator, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            return initHelper(channelCount, internalBufferSize, voiceCount, nullptr, buffer, pitchTranslator, nodeManager, errorState);
        }


        OutputPin* GranulatorInstance::getOutputForChannel(int channel)
        {
            return mChannels[channel]->getOutput();
        }


        void GranulatorInstance::setActiveChannelCount(int channelCount)
        {
            mActiveChannelCount = channelCount;

            for (auto i = 0; i < mChannels.size(); ++i)
                mChannels[i]->setChannelCount(mActiveChannelCount);

            if (!mActive)
                return;

            for (auto i = 0; i < mChannels.size(); ++i)
            {
                if (i < mActiveChannelCount)
                    mChannels[i]->setActive(true);
                else
                    mChannels[i]->setActive(false);
            }

        }


        void GranulatorInstance::setActive(bool active)
        {
            if (active == mActive)
                return;
            mActive = active;
            for (auto i = 0; i < mChannels.size(); ++i)
                if (i < mActiveChannelCount)
                    mChannels[i]->setActive(mActive);
                else
                    mChannels[i]->setActive(false);

        }





    }

}

