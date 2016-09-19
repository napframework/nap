#pragma once

#include "entity.h"

namespace nap
{
    // Track a sibling component dependency
    // Declare this as a component member and pass the parent (this) in initializer list as such:
    //
    // CompDep<Transform> mTransformComp = { this };
    template <typename T>
    class ComponentDependency
    {
    public:
        // Constructor
        ComponentDependency(Component* localComponent)
        {
            mParentComponent = localComponent;
            assert(localComponent);
            RTTI::TypeInfo type = RTTI_OF(T);
            assert(type.isKindOf<Component>());
            
            localComponent->added.connect([=](Object& parent)
            {
                // Wait for addition of our interesting component
                localComponent->getParent()->added.connect(mAddedSlot);

                // Cleanup when the component has been removed
                localComponent->getParent()->removed.connect(mRemovedSlot);

                // If the component was already present, register it.
                if (localComponent->getParent())
                {
                    mTargetComponent = localComponent->getParent()->getComponent<T>();
                    if (mTargetComponent)
                        added(*mTargetComponent);
                }
            });
        }
        
        // Returns the component that the parent depends on
        T* get()
        {
            if (!mTargetComponent)
                mTargetComponent = mParentComponent->getParent()->getComponent<T>();
            return mTargetComponent;
        }
        
        // Overloads
        operator bool() const { return mTargetComponent != nullptr; }
        T* operator ->() { return get(); }
        
        // Signals
        Signal<T&> added;
        Signal<T&> removed;
        
    private:
        void onAdded(Object& comp)
        {
            if (comp.getTypeInfo().isKindOf<T>())
            {
                assert(!mTargetComponent); // Component added a second time, allow only one
                mTargetComponent = static_cast<T*>(&comp);
                added(*mTargetComponent);
            }
        }
        
        void onRemoved(Object& comp)
        {
            if (comp.getTypeInfo().isKindOf<T>())
            {
                removed(*mTargetComponent);
                mTargetComponent = nullptr;
            }
        }
        
        Slot<Object&> mAddedSlot = { this, &ComponentDependency<T>::onAdded };
        Slot<Object&> mRemovedSlot = { this, &ComponentDependency<T>::onRemoved };
        
        T* mTargetComponent = nullptr;
        Component* mParentComponent = nullptr;
    };
    
} //< End Namespace nap
