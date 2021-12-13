/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// nap Includes
#include <sequenceguiservice.h>

namespace nap
{
    /**
     * SequenceAudioGUIService is responsible for registering the appropriate factory functions for AudioTrack views
     * to SequenceGUIService
     */
    class NAPAPI SequenceAudioGUIService final : public Service
    {
    RTTI_ENABLE(Service)
    public:
        /**
         * Colors palette used by all sequencer audio gui
         */
        struct Colors
        {
            /**
             * Initialize palette against SequenceGUIService color palette
             * @param palette ImGUI color palette
             */
            void init(const SequenceGUIService::Colors &palette);

            ImU32 mAudioSegmentBackground = 0; ///< Audio Segment Background Color
            ImU32 mAudioSegmentBackgroundHovering = 0; ///< Audio Segment Background Color when hovering
            ImU32 mAudioSegmentBackgroundClipboard = 0; ///< Audio Segment Background Color when in clipboard
            ImU32 mAudioSegmentBackgroundClipboardHovering = 0; ///< Audio Segment Background Color when in clipboard and hovering
        };

        /**
         * Constructor
         * @param configuration pointer to ServiceConfiguration
         */
        SequenceAudioGUIService(ServiceConfiguration *configuration);

        /**
         * Destructor
         */
        ~SequenceAudioGUIService() override;

        /**
         * Return colors of audio segments
         */
        const SequenceAudioGUIService::Colors &getColors() const
        { return mColors; }

    protected:
        /**
         * registers all objects that need a specific way of construction
         * @param factory the factory to register the object creators with
         */
        void registerObjectCreators(rtti::Factory &factory) override;

        /**
         * initializes service
         * @param errorState contains any errors
         * @return returns true on successful initialization
         */
        bool init(nap::utility::ErrorState &errorState) override;

        /**
         * Override this function to register service dependencies
         * A service that depends on another service is initialized after all it's associated dependencies
         * This will ensure correct order of initialization, update calls and shutdown of all services
         * SequenceAudioGUIService depends on SequenceGUIService and SequenceServiceAudio
         * @param dependencies rtti information of the services this service depends on
         */
        virtual void getDependentServices(std::vector<rtti::TypeInfo> &dependencies);

    private:
        SequenceAudioGUIService::Colors mColors;
    };
}
