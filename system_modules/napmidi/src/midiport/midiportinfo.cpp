/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Nap includes
#include <nap/logger.h>

// External includes
#include "midiportinfo.h"

namespace nap
{

    bool MidiPortInfo::init(utility::ErrorState& errorState)
    {
        try
        {
            mMidiIn = std::make_unique<RtMidiIn>();
            mMidiOut = std::make_unique<RtMidiOut>();
            return true;
        }
        catch (RtMidiError& error)
        {
            errorState.fail(error.getMessage());
            return false;
        }
    }


    int MidiPortInfo::getInputPortCount()
    {
        return mMidiIn->getPortCount();
    }


    std::string MidiPortInfo::getInputPortName(int portNumber)
    {
        return mMidiIn->getPortName(portNumber);
    }


    int MidiPortInfo::getInputPortNumber(const std::string& portName)
    {
        for (auto i = 0; i < mMidiIn->getPortCount(); ++i)
            if (utility::toLower(portName) == utility::toLower(mMidiIn->getPortName(i)))
                return i;
        return -1;
    }


    int MidiPortInfo::getOutputPortCount()
    {
        return mMidiOut->getPortCount();
    }


    std::string MidiPortInfo::getOutputPortName(int portNumber)
    {
        return mMidiOut->getPortName(portNumber);
    }


    int MidiPortInfo::getOutputPortNumber(const std::string& portName)
    {
        for (auto i = 0; i < mMidiOut->getPortCount(); ++i)
            if (utility::toLower(portName) == utility::toLower(mMidiOut->getPortName(i)))
                return i;
        return -1;
    }


    void MidiPortInfo::printPorts()
    {
        Logger::info("Available midi input ports:");
        for (auto i = 0; i < getInputPortCount(); ++i)
            nap::Logger::info("%d: %s", i, getInputPortName(i).c_str());

        Logger::info("Available midi output ports:");
        for (auto i = 0; i < getOutputPortCount(); ++i)
            nap::Logger::info("%d: %s", i, getOutputPortName(i).c_str());
    }

}
