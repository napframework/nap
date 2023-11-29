/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "fftaudionodecomponent.h"
#include "fftbuffer.h"
#include "fftutils.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS(nap::FFTAudioNodeComponent)
	RTTI_PROPERTY("Input",			&nap::FFTAudioNodeComponent::mInput,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Overlaps",		&nap::FFTAudioNodeComponent::mOverlaps,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Channel",		&nap::FFTAudioNodeComponent::mChannel,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FFTAudioNodeComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
RTTI_END_CLASS

namespace nap
{		
	bool FFTAudioNodeComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<FFTAudioNodeComponent>();
		mAudioService = getEntityInstance()->getCore()->getService<audio::AudioService>();
		auto& nodeManager = mAudioService->getNodeManager();
			
		if (!errorState.check(mResource->mChannel < mInput->getChannelCount(), "%s: Channel exceeds number of input channels", mResource->mID.c_str()))
			return false;

		mFFTNode = nodeManager.makeSafe<FFTNode>(nodeManager);
		mFFTNode->mInput.connect(*mInput->getOutputForChannel(mResource->mChannel));
		mFFTBuffer = &mFFTNode->getFFTBuffer();

		return true;
	}


	void FFTAudioNodeComponentInstance::setInput(audio::AudioComponentBaseInstance& input)
	{
		auto inputPtr = &input;
		mAudioService->enqueueTask([&, inputPtr]() {
			mFFTNode->mInput.connect(*inputPtr->getOutputForChannel(mResource->mChannel));
		});
	}
}
