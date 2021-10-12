/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "audiobufferresource.h"

// Audio includes
#include <audio/service/audioservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::AudioBufferResource)
	RTTI_CONSTRUCTOR(nap::audio::AudioService &)
RTTI_END_CLASS

namespace nap
{
	namespace audio
	{
		
		AudioBufferResource::AudioBufferResource(AudioService& service) : mService(service)
		{
			mBuffer = service.getNodeManager().makeSafe<MultiSampleBuffer>();
		}
		
	}
}

