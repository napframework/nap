/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "midiservice.h"
#include "midiinputport.h"
#include "midioutputport.h"
#include "midiinputcomponent.h"

#include <nap/logger.h>
#include <utility/stringutils.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MidiService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	MidiService::MidiService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

    bool MidiService::init(nap::utility::ErrorState& errorState)
    {
        try 
		{
            mMidiIn = std::make_unique<RtMidiIn>();
            mMidiOut = std::make_unique<RtMidiOut>();
            printPorts();
            return true;
        }
        catch (RtMidiError& error)
        {
            errorState.fail(error.getMessage());
            return false;
        }
    }
    
    
    void MidiService::registerObjectCreators(rtti::Factory& factory)
    {
        factory.addObjectCreator(std::make_unique<MidiInputPortObjectCreator>(*this));
        factory.addObjectCreator(std::make_unique<MidiOutputPortObjectCreator>(*this));
    }

    
    int MidiService::getInputPortCount()
    {
        return mMidiIn->getPortCount();
    }
    
    
    std::string MidiService::getInputPortName(int portNumber)
    {
        return mMidiIn->getPortName(portNumber);
    }
    
    
    int MidiService::getInputPortNumber(const std::string& portName)
    {
        for (auto i = 0; i < mMidiIn->getPortCount(); ++i)
            if (utility::toLower(portName) == utility::toLower(mMidiIn->getPortName(i)))
                return i;
        return -1;
    }
    
    
    int MidiService::getOutputPortCount()
    {
        return mMidiOut->getPortCount();
    }
    
    
    std::string MidiService::getOutputPortName(int portNumber)
    {
        return mMidiOut->getPortName(portNumber);
    }
    
    
    int MidiService::getOutputPortNumber(const std::string& portName)
    {
        for (auto i = 0; i < mMidiOut->getPortCount(); ++i)
            if (utility::toLower(portName) == utility::toLower(mMidiOut->getPortName(i)))
                return i;
        return -1;
    }
    
    
    void MidiService::printPorts()
    {
        Logger::info("Available midi input ports:");
        for (auto i = 0; i < getInputPortCount(); ++i)
            nap::Logger::info("%d: %s", i, getInputPortName(i).c_str());

        Logger::info("Available midi output ports:");
        for (auto i = 0; i < getOutputPortCount(); ++i)
            nap::Logger::info("%d: %s", i, getOutputPortName(i).c_str());
    }
    
    
    void MidiService::update(double deltaTime)
    {
        std::unique_ptr<MidiEvent> event = nullptr;
        while (mEventQueue.try_dequeue(event))
            for (auto component : mInputComponents)
            {
                if (!component->mPorts.empty())
                    if (std::find(component->mPorts.begin(), component->mPorts.end(), event->getPort()) == component->mPorts.end())
                        continue;
                if (!component->mTypes.empty())
                    if (std::find(component->mTypes.begin(), component->mTypes.end(), event->getType()) == component->mTypes.end())
                        continue;
                if (!component->mChannels.empty())
                    if (std::find(component->mChannels.begin(), component->mChannels.end(), event->getChannel()) == component->mChannels.end())
                        continue;
                if (!component->mNumbers.empty())
                    if (std::find(component->mNumbers.begin(), component->mNumbers.end(), event->getNumber()) == component->mNumbers.end())
                        continue;
                component->trigger(*event);
            }
    }

    
}
