/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// nap includes
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <sequenceplayeroutput.h>
#include <audio/node/bufferplayernode.h>
#include <audio/node/outputnode.h>
#include <audio/resource/audiobufferresource.h>
#include <audio/service/audioservice.h>
#include <nap/signalslot.h>
#include <rtti/objectptr.h>

// local includes
#include "sequenceplayeraudioadapter.h"

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	// forward declares
	class SequenceService;

	// shortcut to map of created nodes
	using BufferPlayerMap = std::unordered_map<std::string, std::vector<audio::SafeOwner<audio::BufferPlayerNode>>>;

	/**
	 * The SequencePlayerAudioOutput is responsible for translating updates from SequencePlayerAudioAdapters to
	 * appropriate calls to audio::BufferPlayerNodes. When an adapter registers itself to the SequencePlayerAudioOutput
	 * multiple BufferPlayerNodes are created, each adapter gets its own BufferPlayerNode for each AudioBufferResource
	 * and channel.
	 */
	class NAPAPI SequencePlayerAudioOutput : public SequencePlayerOutput
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
		void onDestroy() override ;

		/**
		 * Returns a const reference to a vector of pointers to audio buffer resources
		 * @return const reference to a vector of pointers to audio buffer resources
		 */
		virtual const std::vector<rtti::ObjectPtr<audio::AudioBufferResource>>& getBuffers() const;
	public:
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

		/**
		 * Called when resources are about to be loaded
		 */
		Slot<> mPreResourcesLoadedSlot;
		void onPreResourcesLoaded();

		// pointer to audio service
		audio::AudioService* mAudioService;

		// map of BufferPlayerMaps
		std::unordered_map<const SequencePlayerAudioAdapter*, BufferPlayerMap> mBufferPlayers;

		// map of OutputNodes
		std::unordered_map<const SequencePlayerAudioAdapter*, std::vector<audio::SafeOwner<audio::OutputNode>>> mOutputNodes;

		// object pointers to audio buffers
		std::vector<rtti::ObjectPtr<audio::AudioBufferResource>> mAudioBuffers;
	};

	// shortcut to factory function
	using SequencePlayerAudioOutputObjectCreator = rtti::ObjectCreator<SequencePlayerAudioOutput, SequenceService>;
}