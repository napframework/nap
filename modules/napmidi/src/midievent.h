#pragma once

#include <nap/event.h>

namespace nap {
    
    /**
     * a midi value between 0 an 128
     */
    using MidiValue = short;
    
    
    class NAPAPI MidiEvent : public nap::Event {
    public:
        static constexpr MidiValue MIDI_MAX_VALUE = 127;
        static constexpr MidiValue MIDI_CONTROLLER_SUSTAIN = 64;
        static constexpr MidiValue MIDI_VALUE_OMNI = -1;
        static constexpr MidiValue MIDI_NUMBER_OMNI = MIDI_VALUE_OMNI;
        static constexpr MidiValue MIDI_CHANNEL_OMNI = MIDI_VALUE_OMNI;
        static constexpr MidiValue MIDI_DEVICE_OMNI = MIDI_VALUE_OMNI;
        static constexpr MidiValue MIDI_NUMBER_NONE = 0;
        
        /**
         * Different kinds of midi events
         */
        enum Type { noteOff = 0x80, noteOn = 0x90, afterTouch = 0xA0, controlChange = 0xB0, programChange = 0xC0, channelPressure = 0xD0, pitchBend = 0xE0};

        MidiEvent() = default;
        
        /**
         * Constructor passing in values
         */
        MidiEvent(Type aType, MidiValue number = MIDI_NUMBER_OMNI, MidiValue value = MIDI_VALUE_OMNI, MidiValue channel = MIDI_CHANNEL_OMNI, MidiValue port = MIDI_DEVICE_OMNI);
        
        /**
         * Construct event from raw message data
         */
        MidiEvent(const std::vector<unsigned char>& data, MidiValue port);
        
        virtual ~MidiEvent() = default;
        
        /**
         * this method checks if the messages described with b correspond to this message (using the OMNI wildcards)
         */
        bool corresponds(const MidiEvent &b) const;
        
        bool operator ==(const MidiEvent &rhs) const;
        bool operator >(const MidiEvent &rhs) const { return mNumber > rhs.mNumber; }
        bool operator <(const MidiEvent &rhs) const { return mNumber < rhs.mNumber; }
        
        
        Type mType; // type of the message
        //! this is the note number or the cc number
        MidiValue mNumber;
        //! this is the velocity or the cc value
        MidiValue mValue = MIDI_VALUE_OMNI;
        //! the midi channel through which this message is emitted
        MidiValue mChannel = MIDI_CHANNEL_OMNI;
        //! the port through which the message is emitted
        MidiValue mPort = MIDI_DEVICE_OMNI;
        
        MidiValue noteNumber() const { return mNumber; }
        MidiValue velocity() const { return mValue; }
        MidiValue ccNumber() const { return mNumber; }
        MidiValue ccValue() const { return mValue; }
        float pitchBendValue() const;
        
        std::string getText() const;
        std::vector<unsigned char> getData() const;
    };
    
}
