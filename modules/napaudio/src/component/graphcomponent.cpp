#include "graphcomponent.h"

// Nap includes
#include <nap/entity.h>

// Audio includes
#include <graph/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::GraphComponent)
    RTTI_PROPERTY("Graph", &nap::audio::GraphComponent::mGraph, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GraphComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
    
        bool GraphComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
        {
            auto resource = rtti_cast<GraphComponent>(getComponent());
            return mGraphInstance.init(*resource->mGraph, errorState);
        }
        
        
        OutputPin& GraphComponentInstance::getOutputForChannel(int channel)
        {
            return mGraphInstance.getOutput().getOutputForChannel(channel);
            
        }
        
        
        int GraphComponentInstance::getChannelCount() const
        {
            return mGraphInstance.getOutput().getChannelCount();
            
        }

    }
    
}
