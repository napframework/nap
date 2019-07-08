#pragma once

#include <utility/dllexport.h>
#include <RtMidi.h>
#include <nap/device.h>
#include "midiservice.h"

namespace nap {
    
    /**
     * Opens and manages a midi output port that midi messages can be sent to.
     */
    class NAPAPI MidiOutputPort : public Device
    {
        RTTI_ENABLE(Device)
    public:
        MidiOutputPort() = default;
        MidiOutputPort(MidiService& service);
        
        /**
         * Starts the midi output port.
		 * @param errorState contains the reason why the port could not be opened.
		 * @return if the midi output port opened successfully.
         */
        virtual bool start(utility::ErrorState& errorState) override;

        /**
         * Stops the midi output port.
         */
		virtual void stop() override;
        
		/**
		 * @return the midi device service.
		 */
        MidiService& getService();

        /**
         * Sends a midi event through this output port.
		 * @param event the event to send.
         */
        void sendEvent(const MidiEvent& event);
        
        /**
         * @return: the number of the midi port that messages will be sent through by this object
         */
        int getPortNumber() const;

		std::string mPortName = "";			///< Property: 'Port' The name of the port that midi messages will be sent through.

    private:
        RtMidiOut					mMidiOut;
        MidiService*				mService = nullptr;
        std::vector<unsigned char>	mOutputData;
        int							mPortNumber = -1;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiOutputPortObjectCreator = rtti::ObjectCreator<MidiOutputPort, MidiService>;
    
}
