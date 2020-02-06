#include "graphobject.h"

// Nap includes
#include <entity.h>

// Audio includes
#include <audio/core/audioobject.h>


// RTTI
RTTI_BEGIN_CLASS(nap::audio::GraphObject)
    RTTI_PROPERTY("Graph", &nap::audio::GraphObject::mGraph, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::audio::GraphObjectInstance)
    RTTI_FUNCTION("getObject", &nap::audio::GraphObjectInstance::getObjectNonTyped)
RTTI_END_CLASS


namespace nap
{

    namespace audio
    {

        std::unique_ptr<AudioObjectInstance> GraphObject::createInstance(NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            auto instance = std::make_unique<GraphObjectInstance>();
            if (!instance->init(*mGraph, nodeManager, errorState))
                return nullptr;
            
            return std::move(instance);
        }


        bool GraphObjectInstance::init(Graph& graph, NodeManager& nodeManager, utility::ErrorState& errorState)
        {
            if (!mGraphInstance.init(graph, nodeManager, errorState))
            {
                errorState.fail("Fail to init graph.");
                return false;
            }
            
            return true;
        }
        
        
        OutputPin* GraphObjectInstance::getOutputForChannel(int channel)
        {
            if (mGraphInstance.getOutput() == nullptr)
                return nullptr;
            else
                return mGraphInstance.getOutput()->getOutputForChannel(channel);
        }
        
        
        int GraphObjectInstance::getChannelCount() const
        {
            if (mGraphInstance.getOutput() == nullptr)
                return 0;
            else
                return mGraphInstance.getOutput()->getChannelCount();
        }
        
        
        void GraphObjectInstance::connect(unsigned int channel, OutputPin& pin)
        {
            if (mGraphInstance.getInput() != nullptr)
                mGraphInstance.getInput()->connect(channel, pin);
        }
        
        
        int GraphObjectInstance::getInputChannelCount() const
        {
            if (mGraphInstance.getInput() != nullptr)
                return mGraphInstance.getInput()->getInputChannelCount();
            else
                return 0;
        }

        
    }

}
