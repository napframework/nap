#include "midiinputcomponent.h"

// Nap includes
#include <entity.h>
#include <nap/core.h>

// Midi includes
#include "midiservice.h"

// RTTI
RTTI_BEGIN_CLASS(nap::MidiInputComponent)
    RTTI_PROPERTY("Ports", &nap::MidiInputComponent::mPorts, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Channels", &nap::MidiInputComponent::mChannels, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Numbers", &nap::MidiInputComponent::mNumbers, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Types", &nap::MidiInputComponent::mTypes, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::MidiInputComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
    RTTI_FUNCTION("getMessageReceived", &nap::MidiInputComponentInstance::getMessageReceived)
RTTI_END_CLASS

namespace nap
{
    
    bool MidiInputComponentInstance::init(utility::ErrorState& errorState)
    {
        // Get service and register
        mService = getEntityInstance()->getCore()->getService<MidiService>();
        assert(mService != nullptr);
        mService->registerInputComponent(*this);
     
        // copy event filter settings
        auto resource = getComponent<MidiInputComponent>();
        mPorts = resource->mPorts;
        mChannels = resource->mChannels;
        mNumbers = resource->mNumbers;
        mTypes = resource->mTypes;
        
        return true;
    }
    
    
    MidiInputComponentInstance::~MidiInputComponentInstance()
    {
		if (mService != nullptr)
			mService->unregisterInputComponent(*this);
    }

}
