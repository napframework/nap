#pragma once

// local includes
#include "sequenceplayeroutput.h"

// nap includes
#include <nap/resourceptr.h>
#include <component.h>
#include <audio/node/bufferplayernode.h>
#include <audio/resource/audiofileresource.h>
#include <audio/utility/safeptr.h>
#include <audio/component/audiocomponentbase.h>
#include <audio/node/gainnode.h>
#include <audio/node/controlnode.h>
#include <audio/core/audiopin.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceService;
	class SequencePlayerAudioOutputInstance;
	class SequencePlayerAudioOutputComponent;

	class NAPAPI SequencePlayerAudioOutput : public SequencePlayerOutput
	{
		friend class SequencePlayerAudioAdapter;
		friend class SequencePlayerAudioOutputInstance;

		RTTI_ENABLE(SequencePlayerOutput)
	public:
		/**
		 * Constructor
		 * @param service reference to SequenceService
		 */
		SequencePlayerAudioOutput(SequenceService& service);
	public:
		void addBufferPlayer(audio::AudioFileResource* audioFile);

		void removeBufferPlayer(audio::AudioFileResource* audioFile);

		void clearBufferPlayers();
	protected:
		/**
		 * called from sequence service main thread
		 * @param deltaTime time since last update
		 */
		virtual void update(double deltaTime) override;
	private:
		void registerOutputInstance(SequencePlayerAudioOutputInstance* outputInstance);

		void removeOutputInstance(SequencePlayerAudioOutputInstance* outputInstance);

		std::vector<SequencePlayerAudioOutputInstance*> mOutputs;
	};


	using SequencePlayerAudioOutputObjectCreator = rtti::ObjectCreator<SequencePlayerAudioOutput, SequenceService>;


	class NAPAPI SequencePlayerAudioOutputComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SequencePlayerAudioOutputComponent, SequencePlayerAudioOutputInstance)
	public:
		ResourcePtr<SequencePlayerAudioOutput> mOutput = nullptr;
	};


	class NAPAPI SequencePlayerAudioOutputInstance : public audio::AudioComponentBaseInstance
	{
		RTTI_ENABLE(AudioComponentBaseInstance)
	public:
		SequencePlayerAudioOutputInstance(EntityInstance& entity, Component& resource) : AudioComponentBaseInstance(entity, resource) { }

		// Inherited from ComponentInstance
		bool init(utility::ErrorState& errorState) override;
		void update(double deltaTime) override;

		void onDestroy() override;

		void play(const std::string& id);

		void addBufferPlayer(audio::AudioFileResource* audioFile);

		void removeBufferPlayer(audio::AudioFileResource* audioFile);

		void clearBufferPlayers();

		// Inherited from AudioComponentBaseInstance
		int getChannelCount() const override { return mGainNodes.size(); }
		audio::OutputPin& getOutputForChannel(int channel) override { return mGainNodes[channel]->audioOutput; }
	private:
		std::unordered_map<std::string, audio::SafeOwner<audio::BufferPlayerNode>> mBufferPlayers;

		SequencePlayerAudioOutput* mSequencePlayerOutput = nullptr;
	};
}