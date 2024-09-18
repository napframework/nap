/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "sequenceplayeraudiooutputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Audio includes
#include <audio/service/audioservice.h>
#include <audio/node/outputnode.h>
#include <audio/node/pullnode.h>
#include <audio/component/audiocomponentbase.h>

// RTTI
RTTI_BEGIN_CLASS(nap::audio::SequencePlayerAudioOutputComponent, "Routes output from a sequence player audio output and exposes it to other audio components")
    RTTI_PROPERTY("Sequence Player Audio Output", &nap::audio::SequencePlayerAudioOutputComponent::mSequencePlayerAudioOutput, nap::rtti::EPropertyMetaData::Required, "Sequence player audio output")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::SequencePlayerAudioOutputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance &, nap::Component &)
RTTI_END_CLASS

namespace nap
{
    namespace audio
    {

        bool SequencePlayerAudioOutputComponentInstance::init(utility::ErrorState& errorState)
        {
            auto* resource = getComponent<SequencePlayerAudioOutputComponent>();
            mSequencePlayerAudioOutput = resource->mSequencePlayerAudioOutput.get();

            return true;
        }


        void SequencePlayerAudioOutputComponentInstance::onDestroy()
        {
        }
    }
}
