/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "midiservice.h"

// External Includes
#include <utility/dllexport.h>
#include <nap/device.h>

// Forward Declares
class RtMidiOut;

namespace nap 
{
    
    /**
     * Opens a midi output port that midi messages can be sent to.
	 * Ports are identified by name and assigned a number when opened.
	 * Failure to find and open the midi port is allowed when 'AllowFailure' is set to true.
     */
    class NAPAPI MidiOutputPort : public Device
    {
        RTTI_ENABLE(Device)
    public:
        MidiOutputPort() = default;
        MidiOutputPort(MidiService& service);
		virtual ~MidiOutputPort() override;

        /**
         * Opens the midi output port.
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

		bool mAllowFailure = false;			///< Property: "AllowFailure" failure to find and open port is allowed on startup when set to true.
		std::string mPortName = "";			///< Property: 'Port' The name of the port that midi messages will be sent through.

    private:
        std::unique_ptr<RtMidiOut>	mMidiOut = nullptr;
        MidiService*				mService = nullptr;
        std::vector<unsigned char>	mOutputData;
        int							mPortNumber = -1;
    };
    
    // Object creator used for constructing the the OSC receiver
    using MidiOutputPortObjectCreator = rtti::ObjectCreator<MidiOutputPort, MidiService>;
    
}
