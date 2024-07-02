/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Nap includes
#include <utility/errorstate.h>

// Third party includes
#include <RtMidi.h>

namespace nap
{
    /**
     * Helper class to poll rtmidi for available input and output ports
     */
    class MidiPortInfo final
    {
    public:
        /**
         * Tries to initialize RtMidi
         * @param errorState RtMidi initialization error
         * @return True on success
         */
        bool init(utility::ErrorState& errorState);

        /**
         * Returns number of available midi input ports on the system.
         */
        int getInputPortCount();

        /**
         * Returns the name of a given input port number on this system.
         */
        std::string getInputPortName(int portNumber);

        /**
         * Returns the number of an input port with the given name.
         * Returns -1 when no port with this name has been found.
         */
        int getInputPortNumber(const std::string& portName);

        /**
         * Returns the number of available midi output ports on the system.
         */
        int getOutputPortCount();

        /**
         * Returns the name of a given output port number on this system.
         */
        std::string getOutputPortName(int portNumber);

        /**
         * Returns the number of an output port with the given name.
         * Returns -1 when no port with this name has been found.
         */
        int getOutputPortNumber(const std::string& portName);

        /**
         * Logs a list of all available midi input and output ports on this system.
         */
        void printPorts();

    private:
        std::unique_ptr<RtMidiIn> mMidiIn = nullptr; // used to poll for available input ports
        std::unique_ptr<RtMidiOut> mMidiOut = nullptr; // used to poll available output ports.
    };

}
