#pragma once

#include "nap/objectptr.h"

namespace nap
{
	class Component;
	class ComponentInstance;

	/**
	* ComponentPtr is used in ComponentInstance classes to point to other ComponentInstance objects directly. ComponentInstances are spawned
	* from Components at runtime. The ComponentInstance class makes sure that the internal pointer is mapped to the other spawned object.
	*
	* The Component of a ComponentInstance must hold a regular ObjectPtr to another Component. when an ComponentPtr is contructed, the user
	* should provide the mapping to the ObjectPtr in the Component, by providing the pointer to the member. Example:
	* 
	* 		class SomeComponent : public Component
	*		{
	*			ObjectPtr<OtherComponent> mOtherComponent;
	*		};
	*
	*		class SomeComponentInstance : public ComponentInstance
	*		{
	*			ComponentPtr<OtherComponent> mOtherComponent{ this, &SomeComponent::mOtherComponent };
	*		};
	*
	* In the example above, SomeComponentInstance::mOtherComponent will point the instance that is being pointed to by SomeComponent::mOtherComponent.
	*/
	template<class TargetComponentType>
	class ComponentPtr
	{
	public:
		using TargetComponentInstanceType = typename TargetComponentType::InstanceType;

		template<class SourceComponentType>
		ComponentPtr(ComponentInstance* sourceComponentInstance, ObjectPtr<TargetComponentType>(SourceComponentType::*componentMemberPointer))
		{
			SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
			ObjectPtr<TargetComponentType>& target_component_resource = resource->*componentMemberPointer;

			sourceComponentInstance->addToLinkMap(target_component_resource.get(), (ComponentInstance**)&mInstance);
		}

		const TargetComponentInstanceType& operator*() const
		{
			assert(mInstance != nullptr);
			return *mInstance;
		}

		TargetComponentInstanceType& operator*()
		{
			assert(mInstance != nullptr);
			return *mInstance;
		}

		TargetComponentInstanceType* operator->() const
		{
			assert(mInstance != nullptr);
			return mInstance;
		}

		TargetComponentInstanceType* operator->()
		{
			assert(mInstance != nullptr);
			return mInstance;
		}

		bool operator==(const ComponentPtr<TargetComponentType>& other) const
		{
			return mInstance == other.mPtr;
		}

		template<typename OTHER>
		bool operator==(const ComponentPtr<OTHER>& other) const
		{
			return mInstance == other.mPtr;
		}

		template<typename OTHER>
		bool operator==(const OTHER* ptr) const
		{
			return mInstance == ptr;
		}

		bool operator!=(const ComponentPtr<TargetComponentType>& other) const
		{
			return mInstance != other.mPtr;
		}

		template<typename OTHER>
		bool operator!=(const ComponentPtr<OTHER>& other) const
		{
			return mInstance != other.mPtr;
		}

		template<typename OTHER>
		bool operator!=(const OTHER* ptr) const
		{
			return mInstance != ptr;
		}

		bool operator<(const ComponentPtr<TargetComponentType>& other) const
		{
			return mInstance < other.mInstance;
		}

		bool operator>(const ComponentPtr<TargetComponentType>& other) const
		{
			return mInstance > other.mInstance;
		}

		bool operator<=(const ComponentPtr<TargetComponentType>& other) const
		{
			return mInstance <= other.mInstance;
		}

		bool operator>=(const ComponentPtr<TargetComponentType>& other) const
		{
			return mInstance >= other.mInstance;
		}

		TargetComponentType* get() const
		{
			return mInstance;
		}

		TargetComponentType* get()
		{
			return mInstance;
		}

	private:
		TargetComponentInstanceType* mInstance = nullptr;
	};
}