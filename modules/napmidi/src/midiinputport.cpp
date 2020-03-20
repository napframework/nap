// Local Includes
#include "midiinputport.h"
#include "midievent.h"

// External Includes
#include <nap/logger.h>
#include <RtMidi.h>

RTTI_BEGIN_CLASS(nap::MidiInputPort)
    RTTI_PROPERTY("Ports", &nap::MidiInputPort::mPortNames, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("EnableDebugOutput", &nap::MidiInputPort::mDebugOutput, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    void midiCallback(double deltatime, std::vector<unsigned char> *message, void *userData)
    {
        auto inputPort = static_cast<MidiInputPort*>(userData);
        auto portNames = inputPort->getPortNames();
        auto event = std::make_unique<MidiEvent>(*message, inputPort->mID);
		if (inputPort->mDebugOutput)
		{
			nap::Logger::info("midi input on " + portNames + ": " + event->toString());
		}
        inputPort->receiveEvent(std::move(event));
    }
    
    
    MidiInputPort::MidiInputPort(MidiService& service) :
		mService(&service)
    {

    }


	bool MidiInputPort::start(utility::ErrorState& errorState)
    {
		// No port specified, open all ports, Otherwise find specific port
		if (mPortNames.empty())
		{
			for (auto portNumber = 0; portNumber < mService->getInputPortCount(); ++portNumber)
			{
				mPortNumbers.emplace_back(portNumber);
				mPortNames.emplace_back(mService->getInputPortName(portNumber));
			}
		}
		else
		{
			for (auto& portName : mPortNames)
			{
				auto portNumber = mService->getInputPortNumber(portName);
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

		// Register
		mService->registerInputPort(*this);
		return true;
    }
    

	void MidiInputPort::stop()
	{
		mMidiIns.clear();
		mPortNumbers.clear();
		mPortNames.clear();
		mService->unregisterInputPort(*this);
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
