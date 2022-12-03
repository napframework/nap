/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "midiinputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Midi includes
#include "midiservice.h"

// RTTI
RTTI_BEGIN_CLASS(nap::MidiInputComponent)
    RTTI_PROPERTY("Ports", &nap::MidiInputComponent::mPorts, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Channels", &nap::MidiInputComponent::mChannels, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Numbers", &nap::MidiInputComponent::mNumbers, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("NoteOn", &nap::MidiInputComponent::mNoteOn, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("NoteOff", &nap::MidiInputComponent::mNoteOff, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Aftertouch", &nap::MidiInputComponent::mAfterTouch, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ControlChange", &nap::MidiInputComponent::mControlChange, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ProgramChange", &nap::MidiInputComponent::mProgramChange, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("ChannelPressure", &nap::MidiInputComponent::mChannelPressure, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("PitchBend", &nap::MidiInputComponent::mPitchBend, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MidiInputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getMessageReceived", &nap::MidiInputComponentInstance::getMessageReceived)
RTTI_END_CLASS

namespace nap
{
    
    bool MidiInputComponentInstance::init(utility::ErrorState& errorState)
    {
        // Get service and register
        mService = getEntityInstance()->getCore()->getService<MidiService>();
        assert(mService != nullptr);
        mService->registerInputComponent(*this);
     
        // copy event filter settings
        auto resource = getComponent<MidiInputComponent>();
        mPorts = resource->mPorts;
        mChannels = resource->mChannels;
        mNumbers = resource->mNumbers;

        // copy event type filtering
        if (resource->mNoteOn)
            mTypes.push_back(MidiEvent::Type::noteOn);
        if (resource->mNoteOff)
            mTypes.push_back(MidiEvent::Type::noteOff);
        if (resource->mAfterTouch)
            mTypes.push_back(MidiEvent::Type::afterTouch);
        if (resource->mControlChange)
            mTypes.push_back(MidiEvent::Type::controlChange);
        if (resource->mProgramChange)
            mTypes.push_back(MidiEvent::Type::programChange);
        if (resource->mChannelPressure)
            mTypes.push_back(MidiEvent::Type::channelPressure);
        if (resource->mPitchBend)
            mTypes.push_back(MidiEvent::Type::pitchBend);

        return true;
    }
    
    
    MidiInputComponentInstance::~MidiInputComponentInstance()
    {
		if (mService != nullptr)
			mService->unregisterInputComponent(*this);
    }

}
