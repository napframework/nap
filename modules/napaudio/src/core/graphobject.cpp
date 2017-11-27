#include "graphobject.h"

// Nap includes
#include <entity.h>

// Audio includes
#include <core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::GraphObject)
    RTTI_PROPERTY("Graph", &nap::audio::GraphObject::mGraph, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GraphObjectInstance)
    RTTI_CONSTRUCTOR(nap::audio::GraphObject&)
    RTTI_FUNCTION("getObject", &nap::audio::GraphObjectInstance::getObject)
RTTI_END_CLASS

namespace nap
{
    
    namespace audio
    {
        
        std::unique_ptr<AudioObjectInstance> GraphObject::createInstance()
        {
            return std::make_unique<GraphObjectInstance>(*this);
        }
        
    
        bool GraphObjectInstance::init(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            GraphObject* resource = rtti_cast<GraphObject>(&getResource());
            return mGraphInstance.init(*resource->mGraph, errorState);
        }
        
        
        OutputPin& GraphObjectInstance::getOutputForChannel(int channel)
        {
            return mGraphInstance.getOutput().getOutputForChannel(channel);
        }
        
        
        int GraphObjectInstance::getChannelCount() const
        {
            return mGraphInstance.getOutput().getChannelCount();            
        }

    }
    
}
