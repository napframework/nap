/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "midiinputport.h"
#include <midievent.h>

// External Includes
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::MidiInputPort, "Opens and monitors one or more midi input ports")
    RTTI_PROPERTY("Ports", &nap::MidiInputPort::mPortNames, nap::rtti::EPropertyMetaData::Default, "Names of the ports to open and monitor, empty = all ports")
    RTTI_PROPERTY("EnableDebugOutput", &nap::MidiInputPort::mDebugOutput, nap::rtti::EPropertyMetaData::Default, "Log incoming messages for debug purposes")
RTTI_END_CLASS

namespace nap
{
    void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
    {
        auto inputPort = static_cast<MidiInputPort*>(userData);
        auto event = std::make_unique<MidiEvent>(*message, inputPort->mID);
		if (inputPort->mDebugOutput)
		{
            auto portNames = inputPort->getPortNames();
			nap::Logger::info("midi input on " + portNames + ": " + event->toString());
		}
        inputPort->receiveEvent(std::move(event));
    }
    
    
    MidiInputPort::MidiInputPort(MidiService& service) :
		mService(&service)
    { }


	bool MidiInputPort::start(utility::ErrorState& errorState)
    {
        MidiPortInfo portInfo;
        if (!portInfo.init(errorState))
            return false;

		// No port specified, open all ports, Otherwise find specific port
		if (mPortNames.empty())
		{
			for (auto portNumber = 0; portNumber < portInfo.getInputPortCount(); ++portNumber)
			{
				mPortNumbers.emplace_back(portNumber);
				mPortNames.emplace_back(portInfo.getInputPortName(portNumber));
			}
		}
		else
		{
			for (auto& portName : mPortNames)
			{
				auto portNumber = portInfo.getInputPortNumber(portName);
				if (portNumber < 0)
				{
					errorState.fail("%s: Midi input port not found: %s", mID.c_str(), portName.c_str());
					return false;
				}
				mPortNumbers.emplace_back(portNumber);
			}
		}

		// Try to open all found ports
		for (auto& port : mPortNumbers)
		{
			try
			{
				auto midi_in = std::make_unique<RtMidiIn>();
				midi_in->openPort(port);
				midi_in->setCallback(&midiCallback, this);
				mMidiIns.emplace_back(std::move(midi_in));
			}
			catch (RtMidiError& error)
			{
				errorState.fail("%s: %s", mID.c_str(), error.getMessage().c_str());
				return false;
			}
		}

		return true;
    }
    

	void MidiInputPort::stop()
	{
		mMidiIns.clear();
		mPortNumbers.clear();
		mPortNames.clear();
	}
    

	nap::MidiService& MidiInputPort::getService()
	{
		return *mService;
	}

	
	std::string MidiInputPort::getPortNames() const
    {
        std::string result;
        for (auto& portName : mPortNames)
        {
            result.append(portName);
			if (portName != mPortNames.back())
			{
				result.append(", ");
			}
        }
        return result;
    }
}
