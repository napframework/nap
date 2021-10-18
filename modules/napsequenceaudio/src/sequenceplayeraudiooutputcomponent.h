/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local includes
#include "sequenceplayeraudiooutput.h"

// Nap includes
#include <component.h>
#include <componentptr.h>
#include <audio/utility/safeptr.h>

// Audio includes
#include <audio/component/audiocomponentbase.h>

namespace nap
{
    namespace audio
    {
        class SequencePlayerAudioOutputComponentInstance;

        /**
         * SequencePlayerAudioOutputComponent is an component that routs output from SequencePlayerAudioOutput to the audio interface.
         * When "Create Output Nodes" in SequencePlayerAudioOutput is set to true, this component is not necessary because the
         * SequencePlayerAudioOutput will create its own Output nodes.
         */
        class NAPAPI SequencePlayerAudioOutputComponent : public Component
        {
            RTTI_ENABLE(nap::Component)
            DECLARE_COMPONENT(SequencePlayerAudioOutputComponent, SequencePlayerAudioOutputComponentInstance)
        public:
            /**
             * Constructor
             */
            SequencePlayerAudioOutputComponent() : nap::Component()
            {}

        public:
            ResourcePtr<SequencePlayerAudioOutput> mSequencePlayerAudioOutput; ///< Property: 'Sequence Player Audio Output' resource ptr to audio output of Sequencer
        };


        class NAPAPI SequencePlayerAudioOutputComponentInstance : public ComponentInstance
        {
            RTTI_ENABLE(nap::ComponentInstance)

        public:
            /**
             * Constructor
             * @param entity
             * @param resource
             */
            SequencePlayerAudioOutputComponentInstance(EntityInstance& entity, Component& resource) : nap::ComponentInstance(entity,
                                                                                                          resource)
            {}

            /**
             * onDestroy is called before deconstruction
             */
            void onDestroy() override;

            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

        private:
            // raw pointer to sequence player audio output
            SequencePlayerAudioOutput*          mSequencePlayerAudioOutput;

            // created output nodes
            std::vector<SafeOwner<OutputNode>>  mOutputNodes;
        };

    }
}
