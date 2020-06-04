//
//  spatialspeakersetup.cpp
//  stargate
//
//  Created by Casimir Geelhoed on 04/06/2018.
//
//

#include "MultiSpeakerSetup.h"

// Local includes
#include "MultiSpeakerChannel.h"
#include "SpeakerTest.h"
#include "GridSettingsComponent.h"

// Casipan include
#include "casipan2/Shapes.hpp"

// Tiny XML for XML parsing
#include "tinyxml2.h"

#include "MultiSpeakerService.h"

// Spatial includes
#include <Spatial/Core/SpatialService.h>
#include <Spatial/Core/SpatializationComponent.h>
#include <Spatial/Core/SpatialTypes.h>

// Audio includes
#include <audio/resource/audiofileio.h>
#include <audio/service/audioservice.h>

// Nap includes
#include <nap/logger.h>
#include <mathutils.h>
#include <utility/fileutils.h>
#include <entity.h>

// include for date/time as string
#include <iomanip>


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MultiSpeakerSetupProcess)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MultiSpeakerSetup)
    RTTI_CONSTRUCTOR(nap::spatial::MultiSpeakerService&)
    RTTI_PROPERTY("SpeakerTypeMasters", &nap::spatial::MultiSpeakerSetup::mMasterResources, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::spatial::MonitorSpeakerDescription)
    RTTI_PROPERTY("Name", &nap::spatial::MonitorSpeakerDescription::mName, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Channel", &nap::spatial::MonitorSpeakerDescription::mChannel, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Position",&nap::spatial::MonitorSpeakerDescription::mPosition, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("SpeakerType", &nap::spatial::MonitorSpeakerDescription::mSpeakerType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::spatial::MasterResource)
    RTTI_PROPERTY("Name", &nap::spatial::MasterResource::mName, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("AudioObject", &nap::spatial::MasterResource::mAudioObject, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS


namespace nap
{

    namespace spatial
    {        
        
        MultiSpeakerSetup::MultiSpeakerSetup(MultiSpeakerService& service) : SpeakerSetup(service.getSpatialService()), mMultiSpeakerService(service)
        {
        }
        
        void MultiSpeakerSetupProcess::process()
        {
            switch (mMode.load())
            {
                case audio::ParentProcess::Mode::Parallel:
                    processParallel();
                    break;
                case audio::ParentProcess::Mode::Sequential:
                    processSequential();
                    break;
            }
        }


        void MultiSpeakerSetupProcess::processParallel()
        {
            // The output nodes all access the same output map in the node manager and therefore can not be processed in parallel.
            // The trick is to process the level meter nodes first, so all other audio processing is done already in parallel before the output nodes are processed sequentially.

            // Process the level meter nodes (and the whole speaker setup DSP chain) in parallel
            auto parallelCount = std::min<int>(mThreadPool.getThreadCount(), mChannels.size());
            mAsyncObserver.setBarrier(parallelCount);
            for (auto threadIndex = 0; threadIndex < parallelCount; ++threadIndex)
            {
                mThreadPool.execute([&, threadIndex](){
                    auto i = threadIndex;
                    while (i < mChannels.size())
                    {
                        mChannels[i].mLevelMeterNode->process();
                        i += mThreadPool.getThreadCount();
                    }
                    mAsyncObserver.notifyBarrier();
                });
            }
            mAsyncObserver.waitForNotifications();

            // Just process the output nodes sequentially
            for (auto& channel : mChannels)
            {
                channel.mOutputNode->process();
                channel.mDiskWriterNode->process();
            }
        }


        void MultiSpeakerSetupProcess::processSequential()
        {
            for (auto& channel : mChannels)
            {
                channel.mLevelMeterNode->process();
                channel.mOutputNode->process();
                channel.mDiskWriterNode->process();
            }
        }


        void MultiSpeakerSetupProcess::addChannel(audio::Node& outputNode, audio::Node& levelMeterNode, audio::AudioFileWriterNode& diskWriterNode)
        {
            auto outputNodePtr = &outputNode;
            auto levelMeterNodePtr = &levelMeterNode;
            auto diskWriterNodePtr = &diskWriterNode;
            getNodeManager().enqueueTask([&, outputNodePtr, levelMeterNodePtr, diskWriterNodePtr](){
                mChannels.emplace_back(Channel(*outputNodePtr, *levelMeterNodePtr, *diskWriterNodePtr));
            });
        }


        ParticleProcessor::ParticleProcessor(MultiSpeakerSetup& speakerSetup, SpatializationComponentInstance& soundObject, Particle& particle) : mOwner(soundObject), mParticle(&particle), mSpeakerSetup(&speakerSetup)
        {
            auto& nodeManager = mSpeakerSetup->getSpatialService().getAudioService().getNodeManager();

            int outputCount = mSpeakerSetup->mPanner.getSpeakerCount();
            for(int i = 0; i < outputCount; i++){
                mGainNodes.emplace_back(nodeManager.makeSafe<audio::FastGainNode>(nodeManager));
                mGainNodes.back()->audioInput.enqueueConnect(*particle.getOutputPin());
                mGainNodes.back()->setGain(0.0);
            }


            mPosition = mParticle->getPosition();
            mRotation = mParticle->getRotation();
            mScale = mParticle->getScale();

            // Setup gridSettings
            mGridSettingsComponent = static_cast<GridSettingsComponentInstance*>(soundObject.getEntityInstance()->findComponent("nap::spatial::GridSettingsComponentInstance"));

            if (!mGridSettingsComponent)
                nap::Logger::error("No GridSettingsComponent found at SoundObject.");
            else
                mGridSettingsComponent->getGridSettingsChanged()->connect(mGridSettingsChangedSlot);
            
            pan();

            if (particle.isActive())
                mSpeakerSetup->connectParticleProcessor(*this);

            particle.getTransformChangedSignal()->connect(transformChangedSlot);
            particle.getOutputPinChangedSignal()->connect(outputPinChangedSlot);
            particle.getActiveChangedSignal()->connect(activeChangedSlot);

            soundObject.getDryGainChangedSignal()->connect(gainChangedSlot);

            mParticleActive = particle.isActive();
        }


        void ParticleProcessor::pan()
        {
            mPannerAmplitudes = mSpeakerSetup->mPanner.pan(mPosition, mScale, mRotation, getGridSettings());
            recalculateGains();
        }


        audio::OutputPin& ParticleProcessor::getOutput(int channel)
        {
            return mGainNodes[channel]->audioOutput;
        }
        

        const std::vector<float>& ParticleProcessor::getAmplitudes() const
        {
            return mPannerAmplitudes;
        }
        

        void ParticleProcessor::transformChanged(const SpatialOutput& spatialOutput)
        {

            mPosition = mParticle->getPosition();
            mRotation = mParticle->getRotation();
            mScale = mParticle->getScale();

            pan();
        }


        void ParticleProcessor::outputPinChanged(const SpatialOutput& spatialOutput)
        {
            // ?
        }

        void ParticleProcessor::gainChanged(audio::ControllerValue gain)
        {
            mInputGain = gain;
            recalculateGains();
        }

        void ParticleProcessor::activeChanged(Particle& particle)
        {
            if (particle.isActive()){
                mSpeakerSetup->connectParticleProcessor(*this);
                mParticleActive = true;
            }
            else{
                mSpeakerSetup->disconnectParticleProcessor(*this);
                mParticleActive = false;
            }

        }

        
        std::vector<casipan::GridSettings>& ParticleProcessor::getGridSettings()
        {
            return mGridSettingsComponent->getGridSettings();
        }

        
        void ParticleProcessor::recalculateGains()
        {
            for(int i = 0; i < mGainNodes.size(); i++)
                mGainNodes[i]->setGain(mPannerAmplitudes[i] * mInputGain);
        }


		MultiSpeakerSetup::~MultiSpeakerSetup()
		{
		}

        bool MultiSpeakerSetup::init(utility::ErrorState& errorState)
        {
            auto& multiSpeakerServiceConfig = mMultiSpeakerService.getMultiSpeakerServiceConfiguration();
            auto setupFile = multiSpeakerServiceConfig.mSpeakerSetupFile;
            auto routing = multiSpeakerServiceConfig.mRouting;

            // parse xml
            if (setupFile.empty())
                return true;

            // If the xml parsing fails, it won't crash the application, but display an error in the log instead.
            // It will simply return true but not call onSpeakerSetupCreated at the gui.
            if (!parseXml(setupFile))
                return true;

            auto& audioService = getAudioService();
            int dacChannelCount = audioService.getNodeManager().getOutputChannelCount();
            int channelCount = mPanner.getChannelCount();

            // if routing is not set in JSON, or not enough channels are specified, use straight routing.
            if(routing.size() < channelCount){
                nap::Logger::info("MultiSpeakerSetup: no or invalid routing specified. Using straight routing.");
                routing.clear();
                for(int i = 0; i < channelCount; i++){
                    routing.push_back(i);
                }
                mMultiSpeakerService.setMultiSpeakerRouting(routing);
            }

            // set volumes.
            mMasterVolume = 0.;

            for(int i = 0; i < mGroupNames.size(); i++)
                mGroupVolumes.push_back(1.);


            // make channels.
            for(int i = 0; i < channelCount; i++)
            {
                int outputChannel = routing[i];

                // find corresponding master resource for speakerType
                MasterResource* masterResource = nullptr;
                std::string speakerTypeString = mSpeakerDescriptions[i].mSpeakerType;

                for(int i = 0; i < mMasterResources.size(); i++){
                    if(mMasterResources[i].mName == speakerTypeString)
                        masterResource = &mMasterResources[i];
                }

                auto channel = std::make_unique<MultiSpeakerChannel>();
                channel->init(*this, masterResource, outputChannel, getGroupsForChannel(i), errorState);
                mChannels.emplace_back(std::move(channel));
            }

            // create speakertest
            mSpeakerTest = std::make_unique<SpeakerTest>(getAudioService());

            if (!SpeakerSetup::init(errorState))
            {
                return false;
            }

            return true;
        }



        void MultiSpeakerSetup::update()
        {
        	int index = 0;

        	for (auto& channel : mChannels)
        	{
				MonitorSpeakerDescription & speakerDescription = mSpeakerDescriptions[index];

				// Only read the measured level of the channel if the audio service is actively running. Otherwise the level meter might be containing old data.
				if (getAudioService().isActive())
				    speakerDescription.mMeasuredLevel = channel->getMeasuredLevel();
				else
				    speakerDescription.mMeasuredLevel = 0.f;

        		++index;
			}
		}


        void MultiSpeakerSetup::onDisconnect(){
            if(mSpeakerTestActive)
                setSpeakerTestActive(false);
        }


        bool MultiSpeakerSetup::parseXml(std::string xmlPath){


            tinyxml2::XMLDocument doc;
            tinyxml2::XMLError error = doc.LoadFile(xmlPath.c_str());

            if(error){
                nap::Logger::error("MultiSpeakerSetup couldn't load or parse file: " + xmlPath);
                nap::Logger::error("Error: " + std::string(tinyxml2::XMLDocument::ErrorIDToName(error)));
                return false;

            }

            // get setup
            auto* setup = doc.FirstChildElement("setup");

            // find & add speakers
            for(auto* speakerElement = setup->FirstChildElement("speaker"); speakerElement != NULL; speakerElement = speakerElement->NextSiblingElement("speaker")){

                std::string name = speakerElement->Attribute("id");
                int ch = speakerElement->IntAttribute("ch");
                float x = speakerElement->FloatAttribute("x");
                float y = speakerElement->FloatAttribute("y");
                float z = speakerElement->FloatAttribute("z");

                // prevent tinyXml crash when speakerType is not specified..
                std::string speakerType;
                if(speakerElement->Attribute("speakerType") != NULL)
                    speakerType = speakerElement->Attribute("speakerType");
                else
                    speakerType = "satellite";

                addSpeaker(name, ch, glm::vec3(x,y,z), speakerType);

            }


            // find all grids
            for(auto* gridElement = setup->FirstChildElement("grid"); gridElement != NULL; gridElement = gridElement->NextSiblingElement("grid")){

                // add grid.
                std::string gridName = gridElement->Attribute("name");
                addGrid(gridName);


                // find & add shapes to grid.
                for(auto* shapeElement = gridElement->FirstChildElement("shape"); shapeElement != NULL; shapeElement = shapeElement->NextSiblingElement("shape")){

                    std::string type = shapeElement->Attribute("type");
                    std::string speakersString = shapeElement->Attribute("speakers");
                    std::vector<std::string> speakers;

                    // build speakers vector
                    std::string speakername;
                    std::stringstream speakersStringStream(speakersString);
                    while(speakersStringStream >> speakername)
                        speakers.push_back(speakername);

                    addShapeToGrid(type, speakers, gridName);

                }

            }

            // find all groups.
            for(auto* groupElement = setup->FirstChildElement("group"); groupElement != NULL; groupElement = groupElement->NextSiblingElement("group")){

                std::string name = groupElement->Attribute("name");
                std::string channelsString = groupElement->Attribute("channels");
                std::vector<int> channels;

                int channel;
                std::stringstream speakersStringStream(channelsString);
                while(speakersStringStream >> channel)
                    channels.push_back(channel);

                mGroupNames.push_back(name);
                mGroupChannels.push_back(channels);

            }

            nap::Logger::info("MultiSpeakerSetup loaded gridsetup: " + xmlPath);
            return true;

        }



        void MultiSpeakerSetup::setMasterVolume(float gainValue){
            mMasterVolume = gainValue;
            recalculateAllChannelGains();
        }


        void MultiSpeakerSetup::setGridVolume(int gridIndex, float gainValue){

            mPanner.setGridVolume(gridIndex, gainValue);

            // re-pan all particles
            for(auto& particleProcessor : mParticleProcessors)
                particleProcessor->gridSettingsChanged();

        }


        void MultiSpeakerSetup::setGroupVolume(int groupIndex, float gainValue){
            if(groupIndex < mGroupVolumes.size())
                mGroupVolumes[groupIndex] = gainValue;


            // update relevant gains.
            for(int i = 0; i < mChannels.size(); i++){
                if(mChannels[i]->isMemberOfGroup(groupIndex))
                    recalculateAndSetChannelGain(i);
            }

        }


        void MultiSpeakerSetup::setSpeakerTypeParameter(std::string speakerType, std::string parameterName, float parameterValue){

            // find all channels with same speakerType and set parameter
            for(auto& channel : mChannels){
                if(channel->getSpeakerType() == speakerType)
                    channel->setMasterParameterValue(parameterName, parameterValue);
            }

        }

        void MultiSpeakerSetup::setSpeakerTypeParameterForSpeaker(int speakerIndex, std::string parameterName, float parameterValue){

            if(speakerIndex >= 0 && speakerIndex < mChannels.size())
                mChannels[speakerIndex]->setMasterParameterValue(parameterName, parameterValue);

        }



        void MultiSpeakerSetup::recalculateAndSetChannelGain(int channelIndex){

            std::vector<int>& groupIndexes = mChannels[channelIndex]->getGroupsMembership();

            float gain = 1.;

            gain *= mMasterVolume;

            for(int i = 0; i < groupIndexes.size(); i++)
                gain *= mGroupVolumes[groupIndexes[i]];

            mChannels[channelIndex]->setGain(gain);

        }


        void MultiSpeakerSetup::particleAdded(SpatializationComponentInstance& component, Particle& particle)
        {
            mParticleProcessors.emplace_back(std::make_unique<ParticleProcessor>(*this, component, particle));
        }


        void MultiSpeakerSetup::particleRemoved(SpatializationComponentInstance&, Particle& particle)
        {
            auto particlePtr = &particle;
            auto it = std::find_if(mParticleProcessors.begin(), mParticleProcessors.end(), [particlePtr](auto& panner){ return panner->getParticle() == particlePtr; });
            if (it != mParticleProcessors.end())
                mParticleProcessors.erase(it);
        }



        std::vector<int> MultiSpeakerSetup::getGroupsForChannel(int channel){
            std::vector<int> groups;

            for(int i = 0; i < mGroupNames.size(); i++){
                if(std::find(mGroupChannels[i].begin(), mGroupChannels[i].end(), channel) != mGroupChannels[i].end())
                    groups.push_back(i);
            }
            return groups;
        }

        void MultiSpeakerSetup::connectParticleProcessor(ParticleProcessor& particleProcessor)
        {
            // connect gains to channels
            for(int i = 0; i < mPanner.getSpeakerCount(); i++)
                mChannels[i]->connect(particleProcessor.getOutput(i));
        }


        void MultiSpeakerSetup::disconnectParticleProcessor(ParticleProcessor& particleProcessor)
        {
            // disconnect gains from channels
            for(int i = 0; i < mPanner.getSpeakerCount(); i++)
                mChannels[i]->disconnect(particleProcessor.getOutput(i));
        }


        std::vector<float> MultiSpeakerSetup::getSpeakerAmplitudesForSoundObject(const SpatializationComponentInstance& spatialSoundComponent) const{
            std::vector<float> outputAmps(mPanner.getSpeakerCount(), 0.0);
            for(int i = 0; i < mParticleProcessors.size(); i++){
                if(std::addressof(mParticleProcessors[i]->mOwner) == std::addressof(spatialSoundComponent)){
                    if(mParticleProcessors[i]->mParticleActive){
                        const std::vector<float>& amps = mParticleProcessors[i]->getAmplitudes();
                        for(int j = 0; j < outputAmps.size(); j++)
                            outputAmps[j] += amps[j];
                    }
                }
            }

            return outputAmps;
        }



        void MultiSpeakerSetup::setSpeakerTestActive(bool active){
            mSpeakerTestActive = active;
            if(mSpeakerTestActive)
                mChannels[mSpeakerTestSelectedSpeaker]->connect(mSpeakerTest->getOutput());
            else
                mChannels[mSpeakerTestSelectedSpeaker]->disconnect(mSpeakerTest->getOutput());

        }
        void MultiSpeakerSetup::setSpeakerTestLevel(float level){
            mSpeakerTest->setLevel(level);
        }
        void MultiSpeakerSetup::setSpeakerTestSpeaker(int speaker){
            if(mSpeakerTestActive)
                mChannels[mSpeakerTestSelectedSpeaker]->disconnect(mSpeakerTest->getOutput());

            mSpeakerTestSelectedSpeaker = speaker;

            if(mSpeakerTestActive)
                mChannels[mSpeakerTestSelectedSpeaker]->connect(mSpeakerTest->getOutput());
        }

        void MultiSpeakerSetup::addGrid(std::string name)
        {
            mPanner.addGrid(name);
            mGridNames.push_back(name);
        }

        void MultiSpeakerSetup::addSpeaker(std::string name, int channel, glm::vec3 position, std::string speakerType)
        {
            mPanner.addSpeaker(name, channel, position, speakerType);
            mSpeakerDescriptions.push_back(MonitorSpeakerDescription(name, channel, position, speakerType));

        }

        bool MultiSpeakerSetup::addShapeToGrid(std::string shapeName, std::vector<std::string> speakerNames, std::string gridName)
        {
            if(shapeName == "static")
                mPanner.addShapeToGrid<casipan::StaticShape>(speakerNames, gridName);
            else if(shapeName == "square")
                mPanner.addShapeToGrid<casipan::SquareShape>(speakerNames, gridName);
            else if(shapeName == "triangle")
                mPanner.addShapeToGrid<casipan::TriangleShape>(speakerNames, gridName);
            else if(shapeName == "projectionLine")
                mPanner.addShapeToGrid<casipan::ProjectionLineShape>(speakerNames, gridName);
            else if(shapeName == "cube")
                mPanner.addShapeToGrid<casipan::CubeShape>(speakerNames, gridName);
            else if(shapeName == "projectionPlane")
                mPanner.addShapeToGrid<casipan::ProjectionPlaneShape>(speakerNames, gridName);
            else if(shapeName == "xLine")
                mPanner.addShapeToGrid<casipan::XLineShape>(speakerNames, gridName);
            else if(shapeName == "xStartPoint")
                mPanner.addShapeToGrid<casipan::XStartPointShape>(speakerNames, gridName);
            else if(shapeName == "xEndPoint")
                mPanner.addShapeToGrid<casipan::XEndPointShape>(speakerNames, gridName);
            else if(shapeName == "zLine")
                mPanner.addShapeToGrid<casipan::ZLineShape>(speakerNames, gridName);
            else if(shapeName == "zStartPoint")
                mPanner.addShapeToGrid<casipan::ZStartPointShape>(speakerNames, gridName);
            else if(shapeName == "zEndPoint")
                mPanner.addShapeToGrid<casipan::ZEndPointShape>(speakerNames, gridName);
            else if(shapeName == "deformedCube")
                mPanner.addShapeToGrid<casipan::DeformedCubeShape>(speakerNames, gridName);
            else if(shapeName == "deformedProjectionPlane")
                mPanner.addShapeToGrid<casipan::DeformedProjectionPlaneShape>(speakerNames, gridName);
            else if(shapeName == "projectionTriangle")
                mPanner.addShapeToGrid<casipan::ProjectionTriangleShape>(speakerNames, gridName);
            else if(shapeName == "pyramid")
                mPanner.addShapeToGrid<casipan::PyramidShape>(speakerNames, gridName);
            else if(shapeName == "tetrahedron")
                mPanner.addShapeToGrid<casipan::TetrahedronShape>(speakerNames, gridName);
            else if(shapeName == "triangularPrism")
                mPanner.addShapeToGrid<casipan::TriangularPrismShape>(speakerNames, gridName);
            else if(shapeName == "circle")
                mPanner.addShapeToGrid<casipan::CircleShape>(speakerNames, gridName);
            else if(shapeName == "deformedSquare")
                mPanner.addShapeToGrid<casipan::DeformedSquareShape>(speakerNames, gridName);
            else if(shapeName == "sphere")
                mPanner.addShapeToGrid<casipan::SphereShape>(speakerNames, gridName);
            else
                return false;

            return true;
        }


        audio::SafeOwner<audio::Process> MultiSpeakerSetup::createProcess()
        {
            auto& rootProcess = getSpatialService().getRootProcess();
            auto& nodeManager = rootProcess.getNodeManager();
            return nodeManager.makeSafe<MultiSpeakerSetupProcess>(nodeManager, rootProcess.getThreadPool(), rootProcess.getAsyncObserver());
        }


        bool MultiSpeakerSetup::prependRecording(const std::string &path, utility::ErrorState& errorState)
        {
            mAudioFileWriters.clear();
            
            // get date and time.
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S");
            auto str = oss.str();
            
            // create directory (multi-platform)
            std::string directoryPath = path + "/" + str;			
			if (!utility::makeDirs(directoryPath))
			{
				errorState.fail("Failed to create directory for recording.");
				mAudioFileWriters.clear();
				return false;
			}
            
            for (auto channelNumber = 0; channelNumber < mChannels.size(); ++channelNumber)
            {
                auto audioFile = std::make_unique<audio::AudioFileIO>(getSpatialService().getAudioService().getNodeManager());
                audioFile->mChannelCount = 1;
                audioFile->mPath = directoryPath + "/channel" + std::to_string(channelNumber) + ".wav";
                audioFile->mMode = audio::AudioFileDescriptor::Mode::WRITE;
                if (!audioFile->init(errorState))
                {
                    errorState.fail("Failed to open audio file for recording: %s", audioFile->mPath.c_str());
                    mAudioFileWriters.clear();
                    return false;
                }
                else {
                    mAudioFileWriters.emplace_back(std::move(audioFile));
                }
            }
            return true;
        }


        void MultiSpeakerSetup::startRecording()
        {
            assert(mAudioFileWriters.size() == mChannels.size());
            for (auto channelNumber = 0; channelNumber < mChannels.size(); ++channelNumber)
            {
                mChannels[channelNumber]->startRecording(*mAudioFileWriters[channelNumber]);
            }

        }


        void MultiSpeakerSetup::stopRecording()
        {
            for (auto& channel : mChannels)
                channel->stopRecording();
            mAudioFileWriters.clear(); // Call this to close all files neatly.
        }


        bool MultiSpeakerSetup::prependPlayback(const std::string &path, utility::ErrorState &errorState)
        {
            mAudioFileReaders.clear();
            
            for (auto channelNumber = 0; channelNumber < mChannels.size(); ++channelNumber)
            {
                auto audioFile = std::make_unique<audio::AudioFileIO>(getSpatialService().getAudioService().getNodeManager());
                audioFile->mPath = path + "/channel" + std::to_string(channelNumber) + ".wav";
                audioFile->mMode = audio::AudioFileDescriptor::Mode::READ;
                if (!audioFile->init(errorState))
                {
                    errorState.fail("Failed to open audio file for playback: %s", audioFile->mPath.c_str());
                    mAudioFileReaders.clear();
                    return false;
                }
                else {
                    mAudioFileReaders.emplace_back(std::move(audioFile));
                }
            }
            return true;
        }


        void MultiSpeakerSetup::startPlayback()
        {
            assert(mAudioFileReaders.size() == mChannels.size());
            for (auto channelNumber = 0; channelNumber < mChannels.size(); ++channelNumber)
            {
                mChannels[channelNumber]->startPlayback(*mAudioFileReaders[channelNumber]);
            }

        }


        void MultiSpeakerSetup::stopPlayback()
        {
            for (auto& channel : mChannels)
                channel->stopPlayback();
        }


        void MultiSpeakerSetup::setPlaybackLooping(bool value)
        {
            for (auto& channel : mChannels)
                channel->setPlaybackLooping(value);
        }

        bool MultiSpeakerSetup::isPlayingBack() const
        {
            return mChannels[0]->isPlayingBack();
        }

        
        
    }

}
