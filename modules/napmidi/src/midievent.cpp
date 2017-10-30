#include "midievent.h"

#include <iostream>

RTTI_BEGIN_ENUM(nap::MidiEvent::Type)
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::noteOff, "noteOff"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::noteOn, "noteOn"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::afterTouch, "afterTouch"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::controlChange, "controlChange"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::programChange, "programChange"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::noteOff, "channelPressure"),
    RTTI_ENUM_VALUE(nap::MidiEvent::Type::noteOff, "pitchBend")
RTTI_END_ENUM

namespace nap {
    
    constexpr int noPitchBend = 8192;
    
    
    MidiEvent::MidiEvent(Type aType, MidiValue aNumber, MidiValue aValue, MidiValue aChannel, MidiValue aPort) :
    mType(aType),
    mNumber(aNumber),
    mValue(aValue),
    mChannel(aChannel),
    mPort(aPort)
    {
    }
    
    
    MidiEvent::MidiEvent(const std::vector<unsigned char>& data, MidiValue port) : mPort(port)
    {
        // We need 4 bytes to contain a midi message
        assert(data.size() >= 3);
        
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
                (b.mChannel == mChannel || b.mChannel == MIDI_CHANNEL_OMNI || mChannel == MIDI_CHANNEL_OMNI) &&
                (b.mPort == mPort || b.mPort == MIDI_DEVICE_OMNI || mPort == MIDI_DEVICE_OMNI));
    }
    
    
    bool MidiEvent::operator ==(const MidiEvent &rhs) const
    {
        return ((rhs.mType == mType) &&
                (rhs.mNumber == mNumber) &&
                (rhs.mValue == mValue) &&
                (rhs.mChannel == mChannel) &&
                (rhs.mPort == mPort));
    }
    
    
    float MidiEvent::pitchBendValue() const
    {
        auto x = (mValue << 7) + mNumber;
        
        return (x - noPitchBend) / float(noPitchBend);
    }
    
    
    std::string MidiEvent::getText() const
    {
        std::string result;
        
        switch (mType) {
            case noteOn:
                result = "note on: " + std::to_string(mNumber) + " velocity: " + std::to_string(mValue);
                break;
                
            case noteOff:
                result = "note off: " + std::to_string(mNumber) + " velocity: " + std::to_string(mValue);
                break;
                
            case afterTouch:
                result = "aftertouch: " + std::to_string(mNumber) + " touch: " + std::to_string(mValue);
                break;

            case controlChange:
                result = "cc: " + std::to_string(mNumber) + " value: " + std::to_string(mValue);
                break;

            case programChange:
                result = "program change: " + std::to_string(mNumber);
                break;

            case channelPressure:
                result = "channel pressure: " + std::to_string(mNumber);
                break;
                
            case pitchBend:
                result = "pitch bend: " + std::to_string(pitchBendValue());
                break;
                
            default:
                result = "unknown midi message";
                break;
        }
        
        result = "port: " + std::to_string(mPort) + " channel: " + std::to_string(mChannel) + " " + result;
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
