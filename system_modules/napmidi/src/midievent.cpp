/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "midievent.h"

// Std includes
#include <iostream>

// Nap includes
#include <nap/signalslot.h>

RTTI_BEGIN_ENUM(nap::MidiEvent::Type)
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::noteOff, "noteOff"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::noteOn, "noteOn"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::afterTouch, "afterTouch"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::controlChange, "controlChange"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::programChange, "programChange"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::channelPressure, "channelPressure"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::pitchBend, "pitchBend")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::MidiEvent)
    RTTI_FUNCTION("getType", &nap::MidiEvent::getType)
    RTTI_FUNCTION("getNoteNumber", &nap::MidiEvent::getNoteNumber)
    RTTI_FUNCTION("getVelocity", &nap::MidiEvent::getVelocity)
    RTTI_FUNCTION("getCCNumber", &nap::MidiEvent::getCCNumber)
    RTTI_FUNCTION("getCCValue", &nap::MidiEvent::getCCValue)
    RTTI_FUNCTION("getPitchBendValue", &nap::MidiEvent::getPitchBendValue)
    RTTI_FUNCTION("getPort", &nap::MidiEvent::getPort)
    RTTI_FUNCTION("getChannel", &nap::MidiEvent::getChannel)
    RTTI_FUNCTION("toString", &nap::MidiEvent::toString)
RTTI_END_CLASS

using MidiEventSignal = nap::Signal<const nap::MidiEvent&>;
RTTI_BEGIN_CLASS(MidiEventSignal)
#ifdef NAP_ENABLE_PYTHON
	RTTI_FUNCTION("connect", (void(MidiEventSignal::*)(const pybind11::function))& MidiEventSignal::connect)
#endif // NAP_ENABLE_PYTHON
RTTI_END_CLASS

namespace nap
{
    
    constexpr int noPitchBend = 8192;
    
    
    MidiEvent::MidiEvent(Type aType, MidiValue aNumber, MidiValue aValue, MidiValue aChannel, const std::string& aPort) :
    mType(aType),
    mNumber(aNumber),
    mValue(aValue),
    mChannel(aChannel),
    mPort(aPort)
    {
    }
    
    
    MidiEvent::MidiEvent(const std::vector<unsigned char>& data, const std::string& port) : mPort(port)
    {
        // We only process 3 byte messages for now, midi transport and clock events are not supported yet        
        if (data.size() < 3)
            return;
        
        // extract the type from the first 4 bit of the first byte
        mType = static_cast<Type>((data[0] >> 4) << 4);
        
        // extract the channel from the second 4 bits of the first byte
        mChannel = data[0] - static_cast<int>(mType);
        
        mNumber = data[1];
        mValue = data[2];
    }
    
    
    bool MidiEvent::corresponds(const MidiEvent &b) const
    {
        return ((b.mType == mType) &&
                (b.mNumber == mNumber || b.mNumber == MIDI_NUMBER_OMNI || mNumber == MIDI_NUMBER_OMNI) &&
                (b.mValue == mValue || b.mValue == MIDI_VALUE_OMNI || mValue == MIDI_VALUE_OMNI) &&
                (b.mChannel == mChannel || b.mChannel == MIDI_CHANNEL_OMNI || mChannel ==  MIDI_CHANNEL_OMNI));
    }
    
    
    bool MidiEvent::operator ==(const MidiEvent &rhs) const
    {
        return ((rhs.mType == mType) &&
                (rhs.mNumber == mNumber) &&
                (rhs.mValue == mValue) &&
                (rhs.mChannel == mChannel));
    }
    
    
    float MidiEvent::getPitchBendValue() const
    {
        auto x = (mValue << 7) + mNumber;
        
        return (x - noPitchBend) / float(noPitchBend);
    }
    
    
    std::string MidiEvent::toString() const
    {
        std::string result;
        
        switch (mType) {
            case Type::noteOn:
                result = "note on: " + std::to_string(mNumber) + " velocity: " + std::to_string(mValue);
                break;
                
            case Type::noteOff:
                result = "note off: " + std::to_string(mNumber) + " velocity: " + std::to_string(mValue);
                break;
                
            case Type::afterTouch:
                result = "aftertouch: " + std::to_string(mNumber) + " touch: " + std::to_string(mValue);
                break;

            case Type::controlChange:
                result = "cc: " + std::to_string(mNumber) + " value: " + std::to_string(mValue);
                break;

            case Type::programChange:
                result = "program change: " + std::to_string(mNumber);
                break;

            case Type::channelPressure:
                result = "channel pressure: " + std::to_string(mNumber);
                break;
                
            case Type::pitchBend:
                result = "pitch bend: " + std::to_string(getPitchBendValue());
                break;
                
            default:
                result = "unknown midi message";
                break;
        }
        
        result = result + " channel: " + std::to_string(mChannel);
        return result;
    }

    
    std::vector<unsigned char> MidiEvent::getData() const
    {
        std::vector<unsigned char> result;
        result.emplace_back(static_cast<unsigned char>(mType) + mChannel);
        result.emplace_back(mNumber);
        result.emplace_back(mValue);
        return result;
    }   
}
