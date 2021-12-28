/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/resourcemanager.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>
#include <sequenceserviceaudio.h>
#include <sequencetrackaudio.h>
#include <sequenceserviceaudio.h>

// Local Includes
#include "sequenceaudioguiservice.h"
#include "sequenceaudiotrackview.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SequenceAudioGUIService)
    RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceAudioGUIService*)>& getObjectCreators()
    {
        static std::vector<std::unique_ptr<rtti::IObjectCreator>(*)(SequenceAudioGUIService* service)> vector;
        return vector;
    }


    SequenceAudioGUIService::SequenceAudioGUIService(ServiceConfiguration* configuration)
            :Service(configuration)
    {
    }


    void SequenceAudioGUIService::Colors::init(const SequenceGUIService::Colors& palette)
    {
        // audio segment background
        ImVec4 audio_segment_background_float4 = ImGui::ColorConvertU32ToFloat4(palette.mFro1);
        audio_segment_background_float4.w = 0.25f;
        mAudioSegmentBackground = ImGui::ColorConvertFloat4ToU32(audio_segment_background_float4);

        // audio segment background hovering
        ImVec4 audio_segment_background_hovering_float4 = ImGui::ColorConvertU32ToFloat4(palette.mFro3);
        audio_segment_background_hovering_float4.w = 0.25f;
        mAudioSegmentBackgroundHovering = ImGui::ColorConvertFloat4ToU32(audio_segment_background_hovering_float4);

        // audio segment clipboard
        ImVec4 audio_segment_background_clipboard_float4 = ImGui::ColorConvertU32ToFloat4(palette.mHigh);
        audio_segment_background_clipboard_float4.w = 0.25f;
        mAudioSegmentBackgroundClipboard = ImGui::ColorConvertFloat4ToU32(audio_segment_background_clipboard_float4);

        // audio segment clipboard hovering
        ImVec4 audio_segment_background_hovering_clipboard_float4 = ImGui::ColorConvertU32ToFloat4(palette.mHigh);
        audio_segment_background_hovering_clipboard_float4.w = 0.5f;
        mAudioSegmentBackgroundClipboardHovering = ImGui::ColorConvertFloat4ToU32(audio_segment_background_hovering_clipboard_float4);
    }


    SequenceAudioGUIService::~SequenceAudioGUIService() = default;


    void SequenceAudioGUIService::registerObjectCreators(rtti::Factory& factory)
    {
        for (auto& objectCreator: getObjectCreators())
        {
            factory.addObjectCreator(objectCreator(this));
        }
    }


    bool SequenceAudioGUIService::init(nap::utility::ErrorState& errorState)
    {
        auto* service_gui = getCore().getService<SequenceGUIService>();
        assert(service_gui!=nullptr);

        // init colors base on colors of
        mColors.init(service_gui->getColors());

        // register track view type for SequenceTrackAudio
        if (!errorState.check(service_gui->registerTrackTypeForView(RTTI_OF(SequenceTrackAudio), RTTI_OF(SequenceAudioTrackView)), "Error registering track view"))
            return false;

        // register factory method of SequenceAudioTrackView
        if (!service_gui->registerTrackViewFactory(RTTI_OF(SequenceAudioTrackView), [](SequenceGUIService& service,
                                                                                       SequenceEditorGUIView& editorGuiView,
                                                                                       SequenceEditorGUIState& state) -> std::unique_ptr<SequenceTrackView>
        {
            return std::make_unique<SequenceAudioTrackView>(service, editorGuiView, state);
        }))
        {
            errorState.fail("Error registering track view factory function");
            return false;
        }

        return true;
    }


    void SequenceAudioGUIService::getDependentServices(std::vector<rtti::TypeInfo>& dependencies)
    {
        dependencies.emplace_back(RTTI_OF(SequenceGUIService));
        dependencies.emplace_back(RTTI_OF(SequenceServiceAudio));
    }
}
