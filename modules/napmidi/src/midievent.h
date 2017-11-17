#pragma once

#include <nap/event.h>

namespace nap
{
    
    /**
     * a midi value between 0 an 128
     */
    using MidiValue = short;
    
    
    /**
     * Event representation of a midi message
     * Storing a readable representation of the raw midi message and optionally the port number through which it is sent.
     */
    class NAPAPI MidiEvent : public nap::Event
    {
    public:
        static constexpr MidiValue MIDI_MAX_VALUE = 127;                /**< Maximum value of a midi byte */
        static constexpr MidiValue MIDI_CONTROLLER_SUSTAIN = 64;        /**< CC number for sustain pedal */
        static constexpr MidiValue MIDI_VALUE_OMNI = -1;                /**< Wildcard value for midi values */
        static constexpr MidiValue MIDI_NUMBER_OMNI = MIDI_VALUE_OMNI;  /**< Wildcard value for midi numbers */
        static constexpr MidiValue MIDI_CHANNEL_OMNI = MIDI_VALUE_OMNI; /**< Wildcard value for midi channels */
        static constexpr MidiValue MIDI_PORT_OMNI = MIDI_VALUE_OMNI;    /**< Wildcard value for midi ports */
        static constexpr MidiValue MIDI_NUMBER_NONE = 0;
        
        /**
         * Different kinds of midi events
         */
        enum class Type { noteOff = 0x80, noteOn = 0x90, afterTouch = 0xA0, controlChange = 0xB0, programChange = 0xC0, channelPressure = 0xD0, pitchBend = 0xE0};

        MidiEvent() = default;
        
        /**
         * Constructor passing in values
         */
        MidiEvent(Type aType, MidiValue number = MIDI_NUMBER_OMNI, MidiValue value = MIDI_VALUE_OMNI, MidiValue channel = MIDI_CHANNEL_OMNI, MidiValue port = MIDI_PORT_OMNI);
        
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
        
        
        Type mType;                             /**< The type of the event */
        MidiValue mNumber;                      /**< this is the note number or the cc number */
        MidiValue mValue = MIDI_VALUE_OMNI;     /**< this is the velocity or the cc value */
        MidiValue mChannel = MIDI_CHANNEL_OMNI; /**< the midi channel through which this message is emitted */
        MidiValue mPort = MIDI_PORT_OMNI;       /**< the port through which the message is emitted */
        
        MidiValue noteNumber() const { return mNumber; }    /**< If a note on or off event, this returns the note number */
        MidiValue velocity() const { return mValue; }       /**< If a note on or off event, this returns the velocity */
        MidiValue ccNumber() const { return mNumber; }      /**< If a control change event, this returns the controller number */
        MidiValue ccValue() const { return mValue; }        /**< If a control change event, this returns the controller value */
        float pitchBendValue() const;                       /**< If a pitch bend event, this returns the pitchbend amount between -1 and 1. */
        
        /**
         * Returns the contents of the event as a formatted text string for logging
         */
        std::string getText() const;
        
        /**
         * Returns a raw data midi message that corresponds to the event and can be sent through midi output port.
         */
        std::vector<unsigned char> getData() const;
    };
    
}
