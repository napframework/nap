//
//  multispeakerchannel.cpp
//  testcasi
//
//  Created by Casimir Geelhoed on 14/06/2018.
//
//

#include "MultiSpeakerChannel.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>
#include "MultiSpeakerSetup.h"

// Audio includes
#include <audio/core/audioobject.h>
#include <audio/core/graphobject.h>
#include <audio/resource/audiofileio.h>
#include <audio/service/audioservice.h>

// Nap logger
#include <nap/logger.h>


namespace nap {

    namespace spatial {
    
        MultiSpeakerChannel::~MultiSpeakerChannel()
        {
        }

        
        bool MultiSpeakerChannel::init(MultiSpeakerSetup& speakerSetup, MasterResource* masterResource, int channel, std::vector<int> groupIndexes, utility::ErrorState& errorState)
        {
            mGroupsMembership = groupIndexes;
            
            auto& nodeManager = speakerSetup.getSpatialService().getAudioService().getNodeManager();

            mMixer = nodeManager.makeSafe<audio::FastMixNode>(nodeManager);
            mGain = nodeManager.makeSafe<audio::FastGainNode>(nodeManager, 0.f, FADER_GAINS_RAMP_TIME);

            mGain->audioInput.connect(mMixer->audioOutput);

            if (masterResource != nullptr){
                
                mMaster = masterResource->mAudioObject->instantiate<audio::AudioObjectInstance>(nodeManager, errorState);
                if(mMaster != nullptr){
                    
                    
                    if(mMaster->getInputChannelCount() > 0){
                        
                        // Connect gain output to speaker master input
                        mMaster->connect(0, mGain->audioOutput);
                        mSpeakerType = masterResource->mName;
                        mUsingMaster = true;


                        // Find specific audio objects to enable GUI control.
                        
                        audio::AudioObjectInstance* extraMasterPointer = mMaster.get(); // I'm creating this extra pointer because if I would rtti_cast mMaster itself mMaster becomes a nullptr after the cast.
                        auto masterAsGraphObjectInstance = rtti_cast<audio::GraphObjectInstance>(extraMasterPointer);
                        
                        // find Compressor (to set parameters)
                        if(mSpeakerType == "satellite")
                            mCompressor = masterAsGraphObjectInstance->getObject<audio::ParallelNodeObjectInstance<audio::CompressorNode>>("SatelliteCompressor")->getChannel(0);
                        else if(mSpeakerType == "sub")
                            mCompressor = masterAsGraphObjectInstance->getObject<audio::ParallelNodeObjectInstance<audio::CompressorNode>>("SubCompressor")->getChannel(0);
                        
                        // find LPF (to set parameters)
                        if(mSpeakerType == "sub"){
                            mFilterChain = masterAsGraphObjectInstance->getObject<audio::FilterChainInstance>("SubFilter");
                            mFilterChain->setFrequency(80); // set default.
                        }

                    }
                    else{
                        nap::Logger::info("MultiSpeakerChannel: couldn't find masterchain input.");
                    }
                    
                    
                }
            }
            
            if(!mUsingMaster)
                nap::Logger::info("MultiSpeakerChannel failed to load masterchain.");

            // Create output node
            mOutput = nodeManager.makeSafe<audio::OutputNode>(nodeManager, false);
            mOutput->setOutputChannel(channel);
            if(mUsingMaster)
                mOutput->audioInput.connect(*(mMaster->getOutputForChannel(0)));
            else
                mOutput->audioInput.connect(mGain->audioOutput);
			
            // Add level meter for VU meter output
            audio::OutputPin& outputPin = mUsingMaster ? *mMaster->getOutputForChannel(0) : mGain->audioOutput;
			mLevelMeter = nodeManager.makeSafe<audio::LevelMeterNode>(nodeManager, 50.f, false);
            mLevelMeter->setType(audio::LevelMeterNode::Type::PEAK);
			mLevelMeter->input.connect(outputPin);

			mDiskWriterNode = nodeManager.makeSafe<audio::AudioFileWriterNode>(nodeManager, 4, false);
			mDiskWriterNode->audioInput.connect(mGain->audioOutput);

			mDiskReaderNode = nodeManager.makeSafe<audio::AudioFileReaderNode>(nodeManager, 2048);
			mMixer->inputs.connect(mDiskReaderNode->audioOutput);

            speakerSetup.getProcess<MultiSpeakerSetupProcess>()->addChannel(*mOutput, *mLevelMeter, *mDiskWriterNode);
            
            return true;
            
        }

        
        bool MultiSpeakerChannel::isMemberOfGroup(int index)
        {
            return std::find(mGroupsMembership.begin(), mGroupsMembership.end(), index) != mGroupsMembership.end();
        }
        

        std::vector<int>& MultiSpeakerChannel::getGroupsMembership()
        {
            return mGroupsMembership;
        }
        

        void MultiSpeakerChannel::connect(audio::OutputPin& input)
        {
            mMixer->inputs.enqueueConnect(input);
        }
        

        void MultiSpeakerChannel::disconnect(audio::OutputPin& input)
        {
            mMixer->inputs.enqueueDisconnect(input);
        }
        

        void MultiSpeakerChannel::setMasterParameterValue(std::string name, float value)
        {
            // TODO This should be generic. Implement setParameterValue in GraphObject somehow..?
            // Also another use case for the bigger plan of putting DSP+control together in one block of code.
            if(name == "ratio"){
                if(mCompressor != nullptr)
                    mCompressor->setRatio(value);
            }
            else if(name == "attack"){
                if(mCompressor != nullptr)
                    mCompressor->setAttack(value);
            }
            else if(name == "release"){
                if(mCompressor != nullptr)
                    mCompressor->setRelease(value);
            }
            else if(name == "threshold"){
                if(mCompressor != nullptr)
                    mCompressor->setThreshold(value);
            }
            else if(name == "cutoff"){
                if(mFilterChain != nullptr)
                    mFilterChain->setFrequency(value);
            }
            
        }
                

        audio::OutputNode* MultiSpeakerChannel::getOutputNode()
        {
            return mOutput.getRaw();
        }
        

        audio::LevelMeterNode* MultiSpeakerChannel::getLevelMeterNode()
        {
            return mLevelMeter.getRaw();
        }
        

        void MultiSpeakerChannel::startRecording(audio::AudioFileIO& audioFile)
        {
            mDiskWriterNode->setAudioFile(audioFile.getDescriptor());
        }
        

        void MultiSpeakerChannel::stopRecording()
        {
            mDiskWriterNode->setAudioFile(nullptr);
        }

        
        void MultiSpeakerChannel::startPlayback(audio::AudioFileIO& audioFile)
        {
            audioFile.getDescriptor()->seek(0); // Always start at position 0.
            mDiskReaderNode->setAudioFile(audioFile.getDescriptor());
        }
        

        void MultiSpeakerChannel::stopPlayback()
        {
            mDiskReaderNode->setAudioFile(nullptr);
        }
        

        bool MultiSpeakerChannel::isPlayingBack() const
        {
            return mDiskReaderNode->isPlaying();
        }
        

        void MultiSpeakerChannel::setPlaybackLooping(bool value)
        {
            mDiskReaderNode->setLooping(value);
        }
        
        
        void MultiSpeakerChannel::setGain(float gain){
            mGain->setGain(gain);
        }
		
		
        float MultiSpeakerChannel::getMeasuredLevel() const
        {
        	return mLevelMeter->getLevel();
		}
        
    }
}
