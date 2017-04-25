#pragma once

// Local Includes
#include "entity.h"
#include "logger.h"

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
		ComponentDependency(Component* localComponent, bool required=false);
        
        // Returns the component that the parent depends on, null if not linked
		T* get();
        
        // Overloads
        operator bool() const { return mTargetComponent != nullptr; }
        T* operator ->() { return get(); }
        
        // Signals
        Signal<T&> added;
        Signal<T&> removed;
        
    private:
		// Occurs when a component is added to it's parent
		// Creates a link if of similar target type
		void onAdded(Object& comp);
        
		// Occurs when a component is removed from it's parent
		// Clears the link if of similar target type
		void onRemoved(Object& comp);
        
        Slot<Object&> mAddedSlot = { this, &ComponentDependency<T>::onAdded };
        Slot<Object&> mRemovedSlot = { this, &ComponentDependency<T>::onRemoved };

        T* mTargetComponent = nullptr;
        Component* mParentComponent = nullptr;
        bool mAutoCreate = false;
		bool mRequired;
    };


	//////////////////////////////////////////////////////////////////////////
	// Template Definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	ComponentDependency<T>::ComponentDependency(Component* localComponent, bool required)
	{
		mParentComponent = localComponent;
		mRequired = required;
		assert(localComponent);
		RTTI::TypeInfo type = RTTI_OF(T);
		assert(type.is_derived_from<Component>());

		localComponent->added.connect([=](Object& parent)
		{
			// Wait for addition of our interesting component
			localComponent->getParent()->added.connect(mAddedSlot);

			// Cleanup when the component has been removed
			localComponent->getParent()->removed.connect(mRemovedSlot);

			// Return if it doesn't have a parent
			if (!localComponent->getParent())
				return;

			// Get target component otherwise
			mTargetComponent = localComponent->getParent()->getComponent<T>();
			if (mTargetComponent)
				added(*mTargetComponent);
		});
	}


	template<typename T>
	void ComponentDependency<T>::onAdded(Object& comp)
	{
		if (comp.get_type().is_derived_from<T>())
		{
			assert(!mTargetComponent); // Component added a second time, allow only one
			mTargetComponent = static_cast<T*>(&comp);
			added(*mTargetComponent);
		}
	}


	template<typename T>
	void ComponentDependency<T>::onRemoved(Object& comp)
	{
		if (comp.get_type().is_derived_from<T>())
		{
			removed(*mTargetComponent);
			mTargetComponent = nullptr;
		}
	}


	template<typename T>
	T* ComponentDependency<T>::get()
	{
		// Resolve if not yet linked
		if (!mTargetComponent)
		{
            if (mAutoCreate) {
                mTargetComponent = &mParentComponent->getParent()->getOrCreateComponent<T>();
            } else {
                mTargetComponent = mParentComponent->getParent()->getComponent<T>();
            }
			// Signal change
			if (mTargetComponent) {
                added(*mTargetComponent);
            } else if (mRequired) {
                nap::Logger::fatal("Component of type '%s' requires sibling component of type '%s': '%s'",
                                   mParentComponent->get_type().get_name().data(), RTTI_OF(T).get_name().data(),
					ObjectPath(mParentComponent).toString().c_str());
            }
		}
		return mTargetComponent;
	}
    
} //< End Namespace nap
