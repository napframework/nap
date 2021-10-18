/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// nap includes
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <sequenceplayeroutput.h>
#include <audio/node/outputnode.h>
#include <audio/resource/audiobufferresource.h>
#include <audio/service/audioservice.h>
#include <nap/signalslot.h>
#include <rtti/objectptr.h>
#include <audio/node/mixnode.h>

// local includes
#include "sequenceplayeraudioadapter.h"
#include "multisamplebufferplayernode.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceService;

	// shortcut to map of created multi sample buffer player nodes
	using BufferPlayerMap = std::unordered_map<std::string, audio::SafeOwner<audio::MultiSampleBufferPlayerNode>>;

	/**
	 * The SequencePlayerAudioOutput is responsible for translating updates from SequencePlayerAudioAdapters to
	 * appropriate calls to audio::BufferPlayerNodes. When an adapter registers itself to the SequencePlayerAudioOutput
	 * multiple BufferPlayerNodes are created, each adapter gets its own BufferPlayerNode for each AudioBufferResource
	 * and channel.
	 */
	class NAPAPI SequencePlayerAudioOutput final: public SequencePlayerOutput
	{
		friend class SequencePlayerAudioAdapter;

		RTTI_ENABLE(SequencePlayerOutput);
	public:
		/**
		 * Constructor
		 * @param service reference to service
		 */
		SequencePlayerAudioOutput(SequenceService& service);

		/**
		 * Initialization function
		 * @param errorState contains any errors
		 * @return true on success
		 */
		bool init(utility::ErrorState& errorState) override ;

		/**
		 * called before deconstruction of the resource
		 */
		void onDestroy() override;

		/**
		 * Returns a const reference to a vector of ObjectPointers to audio buffer resources
		 * @return const reference to a vector of ObjectPointers to audio buffer resources
		 */
		virtual const std::vector<rtti::ObjectPtr<audio::AudioBufferResource>>& getBuffers() const;

        /**
         * Connect an inputPin to one of the SequencePlayerAudioOutput channels. Assert when channel is not available.
         * @param inputPin reference to audio::inputPin
         * @param channel the channel to connect to
         */
        void connectInputPin(audio::InputPin& inputPin, int channel);

        /**
         * Disconnect an inputPin from one of the SequencePlayerAudioOutput channels. Assert when channel is not available.
         * @param inputPin reference to audio::inputPin
         * @param channel the channel to connect to
         */
        void disconnectInputPin(audio::InputPin& inputPin, int channel);
	public:
        // properties
        /**
         * When set to true the SequencePlayerAudioOutput will create the necessary output nodes to route audio
         * to the selected playback device by AudioService
         */
        bool mCreateOutputNodes = true;

        /**
         * Maximum output channels, the SequencePlayerAudioOutput cannot play AudioFiles with more then this amount of
         * channels
         */
        int mMaxChannels = 8;
	protected:
		/**
		 * inherited update function, called from sequence service
		 * @param deltaTime
		 */
		void update(double deltaTime) override;
	private:
		/**
		 * Called from a SequencePlayerAudioAdapter
		 * @param adapter pointer to adapter calling the function
		 * @param id the id of the audio segment to play
		 * @param time the time within the segment to play
		 * @param playbackSpeed the playbackspeed
		 */
		void handleAudioSegmentPlay(const SequencePlayerAudioAdapter* adapter, const std::string& id, double time, float playbackSpeed);

		/**
		 * Called from a SequencePlayerAudioAdapter
		 * @param adapter dapter pointer to adapter calling the function
		 * @param id id the id of the audio segment to stop
		 */
		void handleAudioSegmentStop(const SequencePlayerAudioAdapter* adapter, const std::string& id);

		/**
		 * Called when an adapter is created. Multiple BufferPlayerNodes are created upon registration of the adapter
		 * @param adapter pointer to adapter
		 */
		void registerAdapter(const SequencePlayerAudioAdapter* adapter);

		/**
		 * Called when an adapter is destroyed. Any associated BufferPlayerNode will be destroyed as well
		 * @param adapter pointer to adapter
		 */
		void unregisterAdapter(const SequencePlayerAudioAdapter* adapter);

		/**
		 * Called when resources are done loading
		 */
		Slot<> mPostResourcesLoadedSlot;
		void onPostResourcesLoaded();

		// pointer to audio service
		audio::AudioService* mAudioService;

		// map of BufferPlayerMaps
        std::unordered_map<const SequencePlayerAudioAdapter*, BufferPlayerMap> mBufferPlayers;

		// map of output nodes created and owned by SequencePlayerAudioOutput
		std::vector<audio::SafeOwner<audio::OutputNode>> mOutputNodes;

		// object pointers to audio buffers
		std::vector<rtti::ObjectPtr<audio::AudioBufferResource>> mAudioBuffers;

        // mix nodes created and owned by SequencePlayerAudioOutput
        std::vector<audio::SafeOwner<audio::MixNode>> mMixNodes;
	};

	// shortcut to factory function
	using SequencePlayerAudioOutputObjectCreator = rtti::ObjectCreator<SequencePlayerAudioOutput, SequenceService>;
}