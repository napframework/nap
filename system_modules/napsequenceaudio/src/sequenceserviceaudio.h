/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include <sequenceplayer.h>

// External Includes
#include <nap/service.h>
#include <entity.h>
#include <nap/datetime.h>
#include <rtti/factory.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * SequenceServiceAudio registers all necessary factory functions for mod_sequenceaudio to SequenceService
     */
    class NAPAPI SequenceServiceAudio final : public Service
    {
        RTTI_ENABLE(Service)
    public:
        /**
         * Constructor
         */
        SequenceServiceAudio(ServiceConfiguration* configuration);

        /**
         * Deconstructor
         */
        ~SequenceServiceAudio() override;

    protected:
        /**
         * registers all objects that need a specific way of construction
         * @param factory the factory to register the object creators with
         */
        void registerObjectCreators(rtti::Factory& factory) override;

        /**
         * initializes service
         * @param errorState contains any errors
         * @return returns true on successful initialization
         */
        bool init(nap::utility::ErrorState& errorState) override;

        /**
         * updates any outputs and editors
         * @param deltaTime deltaTime
         */
        void update(double deltaTime) override;

        /**
         * SequenceServiceAudio depends on the nap::audio::AudioService
         * @param dependencies the type of services this service depends on
         */
        virtual void getDependentServices(std::vector<rtti::TypeInfo>& dependencies) override;
    };
}
