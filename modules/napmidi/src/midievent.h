/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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
        inline constexpr MidiValue MIDI_MAX_VALUE = 127;				/**< Maximum value of a midi byte */
		inline constexpr MidiValue MIDI_CONTROLLER_SUSTAIN = 64;		/**< CC number for sustain pedal */
		inline constexpr MidiValue MIDI_VALUE_OMNI = -1;				/**< Wildcard value for midi values */
		inline constexpr MidiValue MIDI_NUMBER_OMNI = MIDI_VALUE_OMNI;	/**< Wildcard value for midi numbers */
		inline constexpr MidiValue MIDI_CHANNEL_OMNI = MIDI_VALUE_OMNI;	/**< Wildcard value for midi channels */
		inline constexpr MidiValue MIDI_NUMBER_NONE = 0;
        
        /**
         * Different kinds of midi events
         */
        enum class Type 
		{ 
			noteOff = 0x80,				///< Note off message 
			noteOn = 0x90,				///< Note on message
			afterTouch = 0xA0,			///< After touch message
			controlChange = 0xB0,		///< Control change message
			programChange = 0xC0,		///< Program change message
			channelPressure = 0xD0,		///< Channel pressure message
			pitchBend = 0xE0			///< Pitch band change
		};

        MidiEvent() = default;
        
        /**
         * Constructor passing in values
         */
        MidiEvent(Type aType, MidiValue number = MIDI_NUMBER_OMNI, MidiValue value = MIDI_VALUE_OMNI, MidiValue channel = MIDI_CHANNEL_OMNI, const std::string& port = "");
        
        /**
         * Construct event from raw message data
         */
        MidiEvent(const std::vector<unsigned char>& data, const std::string& port);
        
        virtual ~MidiEvent() = default;
        
        /**
         * this method checks if the messages described with b correspond to this message (using the OMNI wildcards)
         */
        bool corresponds(const MidiEvent &b) const;
        
        bool operator ==(const MidiEvent &rhs) const;
        bool operator >(const MidiEvent &rhs) const { return mNumber > rhs.mNumber; }
        bool operator <(const MidiEvent &rhs) const { return mNumber < rhs.mNumber; }
        
        Type getType() const				{ return mType; }		/**< The type of the event */
        MidiValue getNumber() const			{ return mNumber; }     /**< Returns the first byte: note number, cc number or program number */
        MidiValue getValue() const			{ return mValue; }      /**< Returns the second byte: velocity, CC value */
        MidiValue getNoteNumber() const		{ return mNumber; }		/**< If a note on or off event, this returns the note number */
        MidiValue getVelocity() const		{ return mValue; }      /**< If a note on or off event, this returns the velocity */
        MidiValue getCCNumber() const		{ return mNumber; }     /**< If a control change event, this returns the controller number */
        MidiValue getCCValue() const		{ return mValue; }      /**< If a control change event, this returns the controller value */
        float getPitchBendValue() const;							/**< If a pitch bend event, this returns the pitchbend amount between -1 and 1. */
        MidiValue getProgramNumber() const	{ return mNumber; }		/**< If a program change event, this returns the program number */
        MidiValue getChannel() const		{ return mChannel; }    /**< The midi channel this event is emitted on */
        std::string getPort() const			{ return mPort; }       /**< the mID of the port object through which the message is received, not to be confused with the physical portname */
        
        /**
         * Returns the contents of the event as a formatted text string for logging
         */
        std::string toString() const;
        
        /**
         * Returns a raw data midi message that corresponds to the event and can be sent through midi output port.
         */
        std::vector<unsigned char> getData() const;
        
    private:
        Type mType;                             /**< The type of the event */
        MidiValue mNumber;                      /**< this is the note number or the cc number */
        MidiValue mValue = MIDI_VALUE_OMNI;     /**< this is the velocity or the cc value */
        MidiValue mChannel = MIDI_CHANNEL_OMNI; /**< the midi channel through which this message is emitted */
        std::string mPort = "";                 /**< the mID of the port object through which the message is emitted */
    };
    
}
