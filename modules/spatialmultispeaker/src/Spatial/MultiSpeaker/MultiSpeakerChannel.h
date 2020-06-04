#pragma once 

// Spatial Audio includes
#include <Spatial/Audio/CompressorNode.h>
#include <Spatial/Audio/FilterChain.h>
#include <Spatial/Audio/FastGainNode.h>
#include <Spatial/Audio/FastMixNode.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/node/outputnode.h>
#include <audio/node/pullnode.h>
#include <audio/node/levelmeternode.h>
#include <audio/node/audiofilewriternode.h>
#include <audio/node/audiofilereadernode.h>

// Nap includes
#include <audio/utility/safeptr.h>

// Vector include
#include <vector>
#include <string>


namespace nap {

    namespace utility
    {
        class ErrorState;
    }
    
    namespace audio
    {
        class AudioObjectInstance;
        class FilterChainInstance;
        class AudioFileIO;
        class OutputPin;
    }
    
    namespace spatial {

        
        class MultiSpeakerSetup;
        class MasterResource;
        
        /**
         * The MultiSpeakerChannel represent one output channel (read: speaker) of the MultiSpeakerSetup.
         * It has a separate gain control (that's adjusted by the group & master levels) and can have a custom 'masterchain' based on its speaker type.
         * It also has a LevelMeter that measures the output level for the VU meters.
         */
        class NAPAPI MultiSpeakerChannel
        {
            
        public:
            
            MultiSpeakerChannel() {}
            ~MultiSpeakerChannel();
            
            /**
             * Initialises the MultiSpeakerChannel.
             *
             * @param speakerSetup: the parent MultiSpeakerSetup.
             * @param masterResource: pointer to the @MasterResource of which an instance should be instantiated for this channel.
             * @param channel: DAC output channel number of this MultiSpeakerChannel.
             * @param gridIndex: index of the grid that this speaker is part of (UNUSED since 4dpan2.1).
             * @param groupIndexes: vector of the indexes of the groups that this speaker is part of.
             * @param: errorState: NAP ErrorState.
             */
            bool init(MultiSpeakerSetup& speakerSetup, MasterResource* masterResource, int channel, std::vector<int> groupIndexes, utility::ErrorState& errorState);
            
            
            /**
             * Sets the gain of this channel.
             */
            void setGain(float gain);
			
            
            /**
             * Returns the measured level of this channel.
             */
            float getMeasuredLevel() const;
            
            
            /**
             * Returns whether this channel is a member of the group with index 'index'.
             */
            bool isMemberOfGroup(int index);
            
            /**
             * Returns a vector of all group indexes that this channel is part of.
             */
            std::vector<int>& getGroupsMembership();
            
            /**
             * Connects an @OutputPin to its input.
             */
            void connect(audio::OutputPin& input);
            
            /** 
             * Disconnects an @OutputPin from its input.
             */
            void disconnect(audio::OutputPin& input);
            
            /**
             * Sets a parameter value on its masterchain, using the Node.setParameterValue() function.
             */
            void setMasterParameterValue(std::string name, float value);
            
            /**
             * Returns the SpeakerType string of this channel.
             */
            std::string getSpeakerType(){
                return mSpeakerType;
            }
            
            /**
             * Returns the output node.
             */
            audio::OutputNode* getOutputNode();
            
            /**
             * Returns the level meter node.
             */
            audio::LevelMeterNode* getLevelMeterNode();

            /**
             * Starts recording audio on this channel to audioFile
             */
            void startRecording(audio::AudioFileIO& audioFile);
            
            /**
             * Stops recording on this channel.
             */
            void stopRecording();

            /**
             * Starts playback of an audiofile on this channel
             */
            void startPlayback(audio::AudioFileIO& audioFile);

            /**
             * Stops playback of an audiofile on this channel.
             */

            void stopPlayback();

            /**
             * @return wether this channel is currently playing back an audio file.
             */
            bool isPlayingBack() const;

            /**
             * Sets wether audiofiles played back on this channel will loop.
             */
            void setPlaybackLooping(bool value);

        private:
            std::vector<int> mGroupsMembership; // which groups is this channel part of
            
            std::string mSpeakerType;
            bool mUsingMaster = false;
            
            // dsp chain
            audio::SafeOwner<audio::FastMixNode> mMixer = nullptr;
            audio::SafeOwner<audio::FastGainNode> mGain = nullptr;
            std::unique_ptr<audio::AudioObjectInstance> mMaster = nullptr;

            // disk writer
            audio::SafeOwner<audio::AudioFileWriterNode> mDiskWriterNode = nullptr;

            // disk reader
            audio::SafeOwner<audio::AudioFileReaderNode> mDiskReaderNode = nullptr;

            // master nodes/audioobjects to set parameters (I wish I could set parameters at GraphObject level so this is not necessary and it stays generic)
            audio::CompressorNode* mCompressor = nullptr;
            audio::FilterChainInstance* mFilterChain = nullptr;

            audio::SafeOwner<audio::OutputNode> mOutput = nullptr;
            audio::SafeOwner<audio::LevelMeterNode> mLevelMeter = nullptr;
        };

    }
}
