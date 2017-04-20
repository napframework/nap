// Nap Includes
#include <nap/component.h>
#include <nap/core.h>
#include <nap/logger.h>
#include <nap/entity.h>

// RTTI Define
RTTI_DEFINE_BASE(nap::Component)

namespace nap
{
	// Locks the component
	void nap::Component::lockComponent() { mMutex.lock(); }


	// Unlocks the component
	void Component::unlockComponent() { mMutex.unlock(); }


	Entity* Component::getParent() const
	{
		return static_cast<Entity*>(getParentObject());
	}
    
    
    Entity* Component::getRoot()
    {
        Object* root = getRootObject();
        if (root->get_type().is_derived_from<Entity>())
            return static_cast<Entity*>(root);
        return nullptr;
    }
    

}
