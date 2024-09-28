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
        //////////////////////////////////////////////////////////////////////////

        class SequencePlayerAudioOutputComponentInstance;

        /**
         * SequencePlayerAudioOutputComponent is an component that routes output from SequencePlayerAudioOutput and exposes it
		 * to other AudioComponents.
		 * The component has to be used in combination with an OutputComponent to send the playback to DAC.
         */
        class NAPAPI SequencePlayerAudioOutputComponent : public AudioComponentBase
        {
            RTTI_ENABLE(AudioComponentBase)
            DECLARE_COMPONENT(SequencePlayerAudioOutputComponent, SequencePlayerAudioOutputComponentInstance)
        public:
            /**
             * Constructor
             */
            SequencePlayerAudioOutputComponent()
                    :AudioComponentBase()
            {
            }

            ResourcePtr<SequencePlayerAudioOutput> mSequencePlayerAudioOutput; ///< Property: 'Sequence Player Audio Output' resource ptr to audio output of Sequencer
        };

        class NAPAPI SequencePlayerAudioOutputComponentInstance : public AudioComponentBaseInstance
        {
            RTTI_ENABLE(AudioComponentBaseInstance)

        public:
            /**
             * Constructor
             * @param entity
             * @param resource
             */
            SequencePlayerAudioOutputComponentInstance(EntityInstance& entity, Component& resource)
                    :AudioComponentBaseInstance(entity, resource)
            {
            }


            /**
             * Returns all available channels of output
             * @return all available audio channels
             */
            int getChannelCount() const override
            {
                return mSequencePlayerAudioOutput->getChannelCount();
            }


            /**
             * returns pointer to OutputPin of given channel, assert if out of bounds
             * @param channel channel of OutputPin
             * @return pointer to OutputPin
             */
            OutputPin* getOutputForChannel(int channel) override
            {
                return mSequencePlayerAudioOutput->getOutputForChannel(channel);
            }


            /**
             * onDestroy is called before deconstruction
             */
            void onDestroy() override;

            // Initialize the component
            bool init(utility::ErrorState& errorState) override;

        private:
            // raw pointer to sequence player audio output
            SequencePlayerAudioOutput* mSequencePlayerAudioOutput;
        };
    }
}
