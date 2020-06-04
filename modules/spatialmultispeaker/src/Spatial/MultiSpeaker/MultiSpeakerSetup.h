#pragma once

// Casipan include
#include "casipan2/Panner.hpp"

// Spatial include
#include <Spatial/Core/SpeakerSetup.h>

// Spatial audio includes
#include <Spatial/Audio/FastGainNode.h>

// Audio include
#include <audio/utility/safeptr.h>
#include <audio/core/audioobject.h> // because we can't forward declare ResourcePtr<T> classes
#include <audio/node/audiofilewriternode.h>

// Nap includes
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>

#define FADER_GAINS_RAMP_TIME 10
#define PANNING_GAINS_RAMP_TIME 10

namespace nap
{

    namespace audio
    {
        class AudioFileIO;
    }
    
    namespace spatial
    {

        // Forward declarations
        class SpatialOutput;
        class MultiSpeakerChannel;
        class MultiSpeakerService;
        class SpatializationComponentInstance;
        class GridSettingsComponentInstance;
        class SpeakerTest;


        /**
         * A description of a speaker of a multispeaker setup, containing all relevant information for the visual monitor.
         */
        class NAPAPI MonitorSpeakerDescription {
            RTTI_ENABLE()
        public:
            MonitorSpeakerDescription(std::string name,int channel,glm::vec3 position, std::string speakerType) :
            mName(name), mChannel(channel), mPosition(position), mSpeakerType(speakerType) {}
            virtual ~MonitorSpeakerDescription() { }

            std::string mName;
            int mChannel;
            glm::vec3 mPosition = { 0, 0, 0 };
            std::string mSpeakerType;
            float mMeasuredLevel = 0.f;
            // TODO vector grid membership
            // TODO vector group membership
        };

        /**
         * A DSP chain for a specific speakerType.
         */
        class NAPAPI MasterResource {
            RTTI_ENABLE()

        public:
            MasterResource() = default;
            virtual ~MasterResource() { };
            
            std::string mName; // speakerType name.
            ResourcePtr<audio::AudioObject> mAudioObject = nullptr; // audio object

        };
        
        
        /**
         * The DSP process for the @MultiSpeakerSetup.
         * Parallel processing can be done by processing the channels in parallel.
         */
        class NAPAPI MultiSpeakerSetupProcess : public audio::Process {
            RTTI_ENABLE(Process)
        public:
            /**
             * Data structure specifying the output nodes for one channel.
             */
            struct Channel
            {
                Channel(audio::Node& outputNode, audio::Node& levelMeterNode, audio::Node& diskWriterNode) : mOutputNode(&outputNode), mLevelMeterNode(&levelMeterNode), mDiskWriterNode(&diskWriterNode) {}
                audio::Node* mOutputNode = nullptr;
                audio::Node* mLevelMeterNode = nullptr;
                audio::Node* mDiskWriterNode = nullptr;
            };
        public:
            /**
             * Constructor takes the @NodeManager the process runs on, and the @ThreadPool and @AsyncObserver used for parallelization.
             */
            MultiSpeakerSetupProcess(audio::NodeManager& nodeManager, ThreadPool& threadPool, audio::AsyncObserver& asyncObserver) : audio::Process(nodeManager), mThreadPool(threadPool), mAsyncObserver(asyncObserver) { }
            
            void process() override;
            
            /**
             * Add a channel for processing.
             */
            void addChannel(audio::Node& outputNode, audio::Node& levelMeterNode, audio::AudioFileWriterNode& diskWriterNode);
            
            /**
             * Sets wether the channels will be processed in parallel or sequential.
             */
            void setMode(audio::ParentProcess::Mode mode) { mMode.store(mode); }
            
            /**
             * @return: wether the channels will be processed in parallel or sequential.
             */
            audio::ParentProcess::Mode getMode() const { return mMode.load(); }
            
        private:
            void processParallel();
            void processSequential();
            
            ThreadPool& mThreadPool;
            audio::AsyncObserver& mAsyncObserver;
            std::vector<Channel> mChannels;
            std::atomic<audio::ParentProcess::Mode> mMode = { audio::ParentProcess::Mode::Parallel };
        };
        
        
        // Forward declarations
        class ParticleProcessor;
        class SpeakerTest;

        /**
         * A multispeaker arrangement of speakers that are panned using 4dpan.
         */
        class NAPAPI MultiSpeakerSetup : public SpeakerSetup {

            friend class ParticleProcessor;

            RTTI_ENABLE(SpeakerSetup)
        public:
            MultiSpeakerSetup(MultiSpeakerService& service);
            virtual ~MultiSpeakerSetup() override;

            // Inherited from SpeakerSetup
            bool init(utility::ErrorState& errorState) override;
            void update() override;
            void particleAdded(SpatializationComponentInstance&, Particle&) override;
            void particleRemoved(SpatializationComponentInstance&, Particle&) override;

            /**
             * Sets the master volume.
             */
            void setMasterVolume(float gainValue);

            /**
             * Sets the volume of a grid.
             */
            void setGridVolume(int gridIndex, float gainValue);

            /**
             * Sets the volume of a group.
             */
            void setGroupVolume(int groupIndex, float gainValue);


            /**
             * Returns all relevant information of the speakers for the visual monitor.
             */
            std::vector<MonitorSpeakerDescription>& getSpeakerDescriptions(){ return mSpeakerDescriptions; }

            /**
             * Returns a list of grid names. Called for drawing UI faders.
             */
            std::vector<std::string>& getGridNames(){ return mGridNames; };

            /**
             * Returns a list of group names. Called for drawing UI faders.
             */
            std::vector<std::string>& getGroupNames(){ return mGroupNames; };
            
            std::vector<MasterResource> mMasterResources; ///> property: All masterchains for each speakerType.

            /**
             * Returns "Multi-Speaker Setup"..
             */
			virtual const char * getDisplayName() const override { return "Multi-Speaker"; }

            /**
             * Sums and returns the speaker amplitudes of all particles that belong to the given Sound Object. Used for visualisation.
             */
            std::vector<float> getSpeakerAmplitudesForSoundObject(const SpatializationComponentInstance& spatialSoundComponent) const;

            /**
             * Sets a speakertype-masterchain parameter for all channels with this speaker type.
             */
            void setSpeakerTypeParameter(std::string speakerType, std::string parameterName, float parameterValue);

            /**
             * Sets a speakertype-masterchain parameter for a specific speaker channel.
             */
            void setSpeakerTypeParameterForSpeaker(int speakerIndex, std::string parameterName, float parameterValue);

            /**
             * Enables/disables the speaker test.
             */
            void setSpeakerTestActive(bool active);
            
            /**
             * Adjusts the speaker test level.
             */
            void setSpeakerTestLevel(float level);
            
            /**
             * Sets the active speaker test channel.
             */
            void setSpeakerTestSpeaker(int speaker);

            /**
             * Returns the number of speakers.
             */
            int getSpeakerCount(){ return mPanner.getSpeakerCount(); }

            /**
             * Called when the MultiSpeakerSetup is disconnected. Disables the speaker test if it's still active.
             */
            virtual void onDisconnect() override;

            /**
             * Creates mono audio files for each channel to prepare for recording to disk.
             * @param path Path to a directory where the audio files will be created.
             * @param errorState logs errors when failed to create and open audio files
             * @return true on success
             */
            bool prependRecording(const std::string& path, utility::ErrorState& errorState);

            /**
             * Starts recording to disk. Call @prependRecording prior to this.
             */
            void startRecording();

            /**
             * Stops recording to disk
             */
            void stopRecording();

            /**
             * Opens audio files for each channel to prepare for playback.
             * @param path Path to a directory that contains an audio file for each channel.
             * @param errorState Logs errors while opening audio files.
             * @return true on success.
             */
            bool prependPlayback(const std::string& path, utility::ErrorState& errorState);

            /**
             * Starts playing back a multichannel recording. Call @prependPlayback() prior to this function.
             */
            void startPlayback();

            /**
             * Stops playback of a multichannel recording.
             */
            void stopPlayback();

            /**
             * Set wether multichannel recording playback will be looped.
             */
            void setPlaybackLooping(bool value);

            /**
             * @return wether a multichannel recording is currently being played back. A bit of a hack, as it only checks the state of the first channel.
             */
            bool isPlayingBack() const;

            /**
             * DOC: Stijn.
             */
            audio::SafeOwner<audio::Process> createProcess() override;

        private:
            void connectParticleProcessor(ParticleProcessor& particleProcessor);
            void disconnectParticleProcessor(ParticleProcessor& particleProcessor);

            /**
             * Returns the indexes of the groups that the channel with index 'channel' (pre-routing) is part of.
             */
            std::vector<int> getGroupsForChannel(int channel);

            /**
             * The 4dpan Panner.
             */
            casipan::Panner mPanner;

            /**
             * Adds grids, speakers and shapes to the 4dpan setup according to a setup.xml-file.
             */
            bool parseXml(std::string xmlPath);

            /**
             * Adds a grid to the 4dpan setup. Called by parseXml().
             */
            void addGrid(std::string name);

            /**
             * Adds a speaker to a 4dpan grid. Called by parseXml().
             */
            void addSpeaker(std::string name, int channel, glm::vec3 position, std::string speakerType);

            /**
             * Adds a shape to a 4dpan grid. Called by parseXml().
             */
            bool addShapeToGrid(std::string shapeName, std::vector<std::string> speakerNames, std::string gridName);


            std::vector<MonitorSpeakerDescription> mSpeakerDescriptions; /// Datastructure of all relevant information of the speakers (for the monitor).

            std::vector<std::string> mGridNames; /// List of names of grid (for the UI faders).
            std::vector<std::string> mGroupNames; /// List of names of groups (for the UI faders).

            std::vector<std::vector<int>> mGroupChannels; /// Channels of groups (pre-routing).

            float mMasterVolume; /// Master volume.
            // std::vector<float> mGridVolumes; // Grid volumes.
            std::vector<float> mGroupVolumes; /// Group volumes.

            /**
             * Vector of particle processors. For each connected @Particle, a new @ParticleProcessor is instantiated.
             */
            std::vector<std::unique_ptr<ParticleProcessor>> mParticleProcessors;
            
            /**
             * Vector of MultiSpeakerChannels, representing the output speakers.
             */
            std::vector<std::unique_ptr<MultiSpeakerChannel>> mChannels;

            std::vector<std::unique_ptr<audio::AudioFileIO>> mAudioFileWriters;
            std::vector<std::unique_ptr<audio::AudioFileIO>> mAudioFileReaders;

            /**
             * Recalculates all the channel gains.
             */
            void recalculateAllChannelGains(){
                for(int i = 0; i < mChannels.size(); i++)
                    recalculateAndSetChannelGain(i);
            }
            
            /**
             * Recalculates and sets the channel gain for a specific channel.
             */
            void recalculateAndSetChannelGain(int channelIndex);

            std::unique_ptr<SpeakerTest> mSpeakerTest; /// The speaker test.
            int mSpeakerTestSelectedSpeaker = 0; /// The selected speaker of the speaker test.
            bool mSpeakerTestActive = false; /// Whether the speaker test is active.
            
            MultiSpeakerService& mMultiSpeakerService; ///< Reference to the multi speaker service.
            
        };

        using MultiSpeakerSetupObjectCreator = rtti::ObjectCreator<MultiSpeakerSetup, MultiSpeakerService>;


        /**
         * Helper class to arrange the panning for one @Particle.
         * Connects the Particle's output pin to a @StereoPannerNode and responds to the Particle's signals transformChangedSignal, outputPinChangedSignal and activeChangedSignal.
         * Uses 4dpan to calculate amplitude values for its gains.
         */
        class ParticleProcessor
        {

        public:
            ParticleProcessor(MultiSpeakerSetup& speakerSetup, SpatializationComponentInstance& soundObject, Particle& particle);
            Particle* getParticle() { return mParticle; }

            /**
             * Returns the output for a specific channel.
             */
            audio::OutputPin& getOutput(int channel);
            /**
             * Returns the current amplitudes (used for visualisation).
             */
            const std::vector<float>& getAmplitudes() const;
            
            /**
             * A reference to the SpatialSoundComponent of the Sound Object that owns the Particle connected to this ParticleProcessor.
             */
            SpatializationComponentInstance& mOwner;

            // TODO why? getParticle()->isActive()? And why a public member?
            bool mParticleActive = true; /// Accessible flag for visualisation. Inactive particles should not be displayed.

            /**
             * Re-pans the particle. Is called by GridSettingsSlots when grid settings changed and by MultiSpeakerSetup when a grid volume changed.
             */
            void gridSettingsChanged(){
                pan();
            }

        private:
            void pan();

            Slot<const GridSettingsComponentInstance&> mGridSettingsChangedSlot = { [&](const auto& x){ gridSettingsChanged(); } };
            Slot<const SpatialOutput&> transformChangedSlot = { this, &ParticleProcessor::transformChanged };
            Slot<const SpatialOutput&> outputPinChangedSlot = { this, &ParticleProcessor::outputPinChanged };
            Slot<Particle&> activeChangedSlot = { this, &ParticleProcessor::activeChanged };

            // GridSettingsComponent
            GridSettingsComponentInstance* mGridSettingsComponent;
            
            std::vector<casipan::GridSettings>& getGridSettings();
            
            // input particle gain
            Slot<audio::ControllerValue> gainChangedSlot = { this, &ParticleProcessor::gainChanged };
            void gainChanged(audio::ControllerValue gain);

            void transformChanged(const SpatialOutput&);
            void outputPinChanged(const SpatialOutput&);
            void activeChanged(Particle&);

            void gainLevelsChanged(const std::vector<float>&);
            void boostLevelsChanged(const std::vector<float>&);
            void curvaturesChanged(const std::vector<float>&);
            void projectionPointsChanged(const std::vector<glm::vec3>&);

            void recalculateGains();


            Particle* mParticle = nullptr;
            std::vector<audio::SafeOwner<audio::FastGainNode>> mGainNodes;

            MultiSpeakerSetup* mSpeakerSetup = nullptr;

            glm::vec3 mPosition = glm::vec3(0,0,0);
            glm::vec3 mScale = glm::vec3(0,0,0);
            glm::vec4 mRotation = glm::vec4(0,0,1,0);


            float mInputGain = 1.;
            std::vector<float> mPannerAmplitudes;

        };

    }

}
