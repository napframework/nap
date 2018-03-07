#include "graph.h"


// Nap includes
#include <rtti/rttiutilities.h>
#include <nap/objectgraph.h>



// RTTI
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::Graph)
    RTTI_CONSTRUCTOR(nap::audio::NodeManager&)
    RTTI_PROPERTY("Objects", &nap::audio::Graph::mObjects, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Output", &nap::audio::Graph::mOutput, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::audio::GraphInstance)
    RTTI_FUNCTION("getObject", &nap::audio::GraphInstance::getObject)
RTTI_END_CLASS


namespace nap
{
    
    namespace audio
    {
        
        
        /**
         * Item class for ObjectGraph usage.
         */
        class AudioGraphItem
        {
        public:
            using Type = AudioObject*;
            
            /**
             * Creates a graph item.
             * @param object Object to wrap in the item that is created.
             */
            static const AudioGraphItem create(AudioObject* object)
            {
                AudioGraphItem item;
                item.mObject = object;
                return item;
            }
            
            /**
             * @return ID of the item.
             */
            const std::string getID() const
            {
                return mObject->mID;
            }
            
            /**
             * Performs rtti traversal of pointers to AudioObject.
             * @param pointees Output parameter, contains all objects and files this object points to.
             * @param errorState If false is returned, contains information about the error.
             * @return true is succeeded, false otherwise.
             */
            bool getPointees(std::vector<AudioGraphItem>& pointees, utility::ErrorState& errorState) const
            {
                std::vector<rtti::ObjectLink> object_links;
                rtti::findObjectLinks(*mObject, object_links);
                
                for (const rtti::ObjectLink& link : object_links)
                {
                    if (link.mTarget == nullptr)
                        continue;
                    
                    if (link.mTarget->get_type().is_derived_from<AudioObject>())
                    {
                        AudioGraphItem item;
                        item.mObject = rtti_cast<AudioObject>(link.mTarget);
                        pointees.push_back(item);
                    }
                }
                
                return true;
            }
            
            AudioObject* mObject = nullptr;
        };

        
        bool GraphInstance::init(Graph& resource, utility::ErrorState& errorState)
        {
            mResource = &resource;
            
            // Build object graph as utility to sort all the audio object resources in dependency order
            std::vector<AudioObject*> objects;
            for (auto& object : resource.mObjects)
                objects.emplace_back(object.get());
            
            ObjectGraph<AudioGraphItem> graph;
            if (!graph.build(objects, [](AudioObject* object) { return AudioGraphItem::create(object); }, errorState))
            {
                errorState.fail("Failed to build audio graph %s", resource.mID.c_str());
                return false;
            }
            
            // Sort in order of depenedency
            auto sortedNodes = graph.getSortedNodes();
            
            for (auto& node : sortedNodes)
            {
                // Create instance and initialize
                auto objectResource = node->mItem.mObject;
                auto instance = objectResource->instantiate(resource.getNodeManager(), errorState);
                
                if (instance == nullptr)
                {
                    errorState.fail("Failed to instantiate graph %s", resource.mID.c_str());
                    return false;
                }
                
                if (objectResource == resource.mOutput.get())
                    mOutput = instance.get();
                mObjects.emplace_back(std::move(instance));
            }
            
            if (mOutput == nullptr)
            {
                errorState.fail("%s Graph output not found within the graph.", resource.mID.c_str());
                return false;
            }
            
            return true;
        }
        
        
        AudioObjectInstance* GraphInstance::getObject(const std::string &mID)
        {
            for (auto& object : mObjects)
                if (object->getResource().mID == mID)
                    return object.get();
            return nullptr;
        }
        
    }
    
}
