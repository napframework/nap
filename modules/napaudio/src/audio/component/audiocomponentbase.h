#pragma once

// Nap includes
#include <component.h>
#include <componentptr.h>
#include <rtti/objectptr.h>

// Audio includes
#include <audio/core/audionode.h>
#include <audio/core/multichannel.h>

namespace nap
{
	namespace audio
	{
		
		// Forward declarations
		class AudioService;
		
		class AudioComponentBaseInstance;
		
		
		/**
		 * Component that generates audio output for one or more channels.
		 * Note: does not send the audio to de DAC yet, in order to do this add an OutputComponent as well.
		 */
		class NAPAPI AudioComponentBase : public Component
		{
			RTTI_ENABLE(nap::Component)
			DECLARE_COMPONENT(AudioComponent, AudioComponentBaseInstance)
		
		public:
			AudioComponentBase() : nap::Component()
			{}
		
		private:
		};
		
		
		/**
		 * Instance of a component that generates audio output for one or more channels.
		 */
		class NAPAPI AudioComponentBaseInstance : public ComponentInstance, public IMultiChannelOutput
		{
		RTTI_ENABLE(nap::ComponentInstance)
		
		public:
			AudioComponentBaseInstance(EntityInstance& entity, Component& resource);
		
		protected:
			/**
			 * @return the node system's node manager that the audio runs on
			 */
			NodeManager& getNodeManager();
			
			/**
			 * Returns the audio service
			 */
			AudioService& getAudioService() { return *mAudioService; }
		
		private:
			AudioService* mAudioService = nullptr;
		};
		
	}
}
