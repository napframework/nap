/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiobufferresource.h"

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioBufferResource)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{

        AudioBufferResource::AudioBufferResource(nap::Core& core)
		{
            auto audioService = core.getService<AudioService>();
			mBuffer = audioService->getNodeManager().makeSafe<MultiSampleBuffer>();
		}
		
	}
}

