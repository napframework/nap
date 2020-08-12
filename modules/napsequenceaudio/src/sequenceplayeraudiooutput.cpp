#include "sequenceplayeraudiooutput.h"
#include "sequenceservice.h"
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioOutput)
RTTI_CONSTRUCTOR(nap::SequenceService&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::SequencePlayerAudioOutputComponent)
RTTI_PROPERTY("Output", &nap::SequencePlayerAudioOutputComponent::mOutput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequencePlayerAudioOutputInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

using namespace nap::audio;

namespace nap
{
	static bool registerObjectCreator = SequenceService::registerObjectCreator([](SequenceService* service)->std::unique_ptr<rtti::IObjectCreator>
	{
		return std::make_unique<SequencePlayerAudioOutputObjectCreator>(*service);
	});


	SequencePlayerAudioOutput::SequencePlayerAudioOutput(SequenceService& service)
		: SequencePlayerOutput(service) {}


	void SequencePlayerAudioOutput::update(double deltaTime)
	{
	}


	void SequencePlayerAudioOutput::addBufferPlayer(audio::AudioFileResource* audioFile)
	{
		for (auto& output : mOutputs)
		{
			output->addBufferPlayer(audioFile);
		}
	}


	void SequencePlayerAudioOutput::removeBufferPlayer(audio::AudioFileResource* audioFile)
	{
		for (auto& output : mOutputs)
		{
			output->removeBufferPlayer(audioFile);
		}
	}


	void SequencePlayerAudioOutput::clearBufferPlayers()
	{
		for (auto& output : mOutputs)
		{
			output->clearBufferPlayers();
		}
	}


	void SequencePlayerAudioOutput::registerOutputInstance(SequencePlayerAudioOutputInstance* outputInstance)
	{
		auto found_it = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& it)
		{
			return it == outputInstance;
		});
		assert(found_it == mOutputs.end()); // duplicate entry

		if (found_it == mOutputs.end())
		{
			mOutputs.emplace_back(outputInstance);
		}
	}

	void SequencePlayerAudioOutput::removeOutputInstance(SequencePlayerAudioOutputInstance* outputInstance)
	{
		auto found_it = std::find_if(mOutputs.begin(), mOutputs.end(), [&](const auto& it)
		{
			return it == outputInstance;
		});
		assert(found_it != mOutputs.end()); // output is not registered

		if (found_it != mOutputs.end())
		{
			mOutputs.erase(found_it);
		}
	}


	bool SequencePlayerAudioOutputInstance::init(utility::ErrorState &errorState)
	{
		//
		mSequencePlayerOutput = getComponent<SequencePlayerAudioOutputComponent>()->mOutput.get();
		mSequencePlayerOutput->registerOutputInstance(this);

		return true;
	}


	void SequencePlayerAudioOutputInstance::update(double deltaTime)
	{

	}


	void SequencePlayerAudioOutputInstance::addBufferPlayer(AudioFileResource* audioFile)
	{
		auto& nodeManager = getAudioService().getNodeManager();
		auto bufferPlayer = getAudioService().makeSafe<BufferPlayerNode>(nodeManager);

		bufferPlayer->setBuffer(audioFile->getBuffer());

		mBufferPlayers.emplace(audioFile->mID, bufferPlayer);
	}


	void SequencePlayerAudioOutputInstance::removeBufferPlayer(AudioFileResource* audioFile)
	{
		auto it = mBufferPlayers.find(audioFile->mID);
		if (it != mBufferPlayers.end())
		{
			mBufferPlayers.erase(it);
		}
	}


	void SequencePlayerAudioOutputInstance::play(const std::string& id)
	{
		auto it = mBufferPlayers.find(id);
		if (it != mBufferPlayers.end())
		{
			it->second->play();
		}
	}


	void SequencePlayerAudioOutputInstance::clearBufferPlayers()
	{
		mBufferPlayers.clear();
	}


	void SequencePlayerAudioOutputInstance::onDestroy()
	{
		mSequencePlayerOutput->removeOutputInstance(this);
	}
}