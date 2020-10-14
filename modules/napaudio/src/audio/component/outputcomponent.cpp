#include "outputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/node/outputnode.h>
#include <audio/node/pullnode.h>
#include "audiocomponentbase.h"

// RTTI
RTTI_BEGIN_CLASS(nap::audio::OutputComponent)
		RTTI_PROPERTY("Input", &nap::audio::OutputComponent::mInput, nap::rtti::EPropertyMetaData::Required)
		RTTI_PROPERTY("Routing", &nap::audio::OutputComponent::mChannelRouting, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::OutputComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		bool OutputComponentInstance::init(utility::ErrorState& errorState)
		{
			OutputComponent* resource = getComponent<OutputComponent>();
			
			mAudioService = getEntityInstance()->getCore()->getService<AudioService>();
			auto& nodeManager = mAudioService->getNodeManager();
			
			auto channelCount = resource->mChannelRouting.size();
			if (channelCount > nodeManager.getOutputChannelCount() && !mAudioService->getAllowChannelCountFailure()) {
				errorState.fail("%s: Trying to rout to output channel that is out of bounds.", resource->mID.c_str());
				return false;
			}
			
			for (auto channel = 0; channel < channelCount; ++channel)
				if (resource->mChannelRouting[channel] >= mInput->getChannelCount())
				{
					errorState.fail("%s: Trying to rout input channel that is out of bounds.", resource->mID.c_str());
					return false;
				}
			
			for (auto channel = 0; channel < channelCount; ++channel)
			{
				if (resource->mChannelRouting[channel] < 0)
					continue;
				
				if (channel >= nodeManager.getOutputChannelCount())
				{
					// If the channel is out of bounds we create a PullNode instead of an OutputNode in order to process the connected DSP branch.
					auto pullNode = nodeManager.makeSafe<PullNode>(nodeManager);
					pullNode->audioInput.connect(*mInput->getOutputForChannel(resource->mChannelRouting[channel]));
					mOutputs.emplace_back(std::move(pullNode));
					continue;
				}
				else {
					auto outputNode = nodeManager.makeSafe<OutputNode>(nodeManager);
					outputNode->setOutputChannel(channel);
					outputNode->audioInput.connect(*mInput->getOutputForChannel(resource->mChannelRouting[channel]));
					mOutputs.emplace_back(std::move(outputNode));
				}
			}
			
			return true;
		}
		
		
		void OutputComponentInstance::setInput(AudioComponentBaseInstance& input)
		{
			OutputComponent* resource = getComponent<OutputComponent>();
			
			AudioComponentBaseInstance* inputPtr = &input;
			mAudioService->enqueueTask([&, inputPtr, resource]() {
				auto channelCount = resource->mChannelRouting.size();
				for (auto channel = 0; channel < channelCount; ++channel)
				{
					auto outputNode = mOutputs[channel].getRaw();
					
					//
					int inputChannel = resource->mChannelRouting[channel] % inputPtr->getChannelCount();
					
					// in case of a normal output node
					if (outputNode->get_type().is_derived_from(RTTI_OF(OutputNode)))
						static_cast<OutputNode*>(outputNode)->audioInput.connect(*inputPtr->getOutputForChannel(inputChannel));
					
					// in case this actual output channel is not available on the device we are dealing with a pulling node
					if (outputNode->get_type().is_derived_from(RTTI_OF(PullNode)))
						static_cast<PullNode*>(outputNode)->audioInput.connect(*inputPtr->getOutputForChannel(inputChannel));
				}
			});
		}
		
	}
}
