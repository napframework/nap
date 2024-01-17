/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "midiservice.h"
#include "midiinputcomponent.h"

#ifdef NAP_ENABLE_RTMIDI
    #include "midiport/midiinputport.h"
    #include "midiport/midioutputport.h"
    #include "midiport/midiportinfo.h"
#endif

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
#ifdef NAP_ENABLE_RTMIDI
        // When RtMdidi is enabled, try to initialize it and poll for available ports
        MidiPortInfo portInfo;
        if (!portInfo.init(errorState))
            return false;

        // Print available midi ports
        portInfo.printPorts();
#endif
        return true;
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
