#pragma once

#include "nap/objectptr.h"

namespace nap
{
	class Component;
	class ComponentInstance;

	class ComponentPtrBase
	{
		RTTI_ENABLE();

	private:
		friend class ResourceManagerService;

		static std::string translateTargetID(const std::string& targetID);
		virtual void setValue(const std::string& path, rtti::RTTIObject* pointer) = 0;
	};

	template<class ComponentType>
	class ComponentPtr : public ComponentPtrBase
	{
		RTTI_ENABLE(ComponentPtrBase)

	public:
		ComponentPtr() = default;

		ComponentPtr(ComponentType* component) :
			mResource(component)
		{
		}

		const std::string& getInstancePath() const { return mPath; }

		virtual void setValue(const std::string& path, rtti::RTTIObject* pointer) override
		{
			mPath = path;
			mResource = rtti_cast<ComponentType>(pointer);
		}

		const ComponentType& operator*() const
		{
			assert(mResource != nullptr);
			return *mResource;
		}

		ComponentType& operator*()
		{
			assert(mResource != nullptr);
			return *mResource;
		}

		ComponentType* operator->() const
		{
			assert(mResource != nullptr);
			return mResource;
		}

		ComponentType* operator->()
		{
			assert(mResource != nullptr);
			return mResource;
		}

		bool operator==(const ComponentPtr<ComponentType>& other) const
		{
			return mResource == other.mResource;
		}

		template<typename OTHER>
		bool operator==(const ComponentPtr<OTHER>& other) const
		{
			return mResource == other.mResource;
		}

		template<typename OTHER>
		bool operator==(const OTHER* ptr) const
		{
			return mResource == ptr;
		}

		bool operator==(std::nullptr_t) const
		{
			return mResource == nullptr;
		}

		bool operator!=(const ComponentPtr<ComponentType>& other) const
		{
			return mResource != other.mResource;
		}

		template<typename OTHER>
		bool operator!=(const ComponentPtr<OTHER>& other) const
		{
			return mResource != other.mResource;
		}

		template<typename OTHER>
		bool operator!=(const OTHER* ptr) const
		{
			return mResource != ptr;
		}

		bool operator!=(std::nullptr_t) const
		{
			return mResource != nullptr;
		}

		bool operator<(const ComponentPtr<ComponentType>& other) const
		{
			return mResource < other.mResource;
		}

		bool operator>(const ComponentPtr<ComponentType>& other) const
		{
			return mResource > other.mResource;
		}

		bool operator<=(const ComponentPtr<ComponentType>& other) const
		{
			return mResource <= other.mResource;
		}

		bool operator>=(const ComponentPtr<ComponentType>& other) const
		{
			return mResource >= other.mResource;
		}

		ComponentType* get() const
		{
			return mResource.get();
		}

		ComponentType* get()
		{
			return mResource.get();
		}

	private:
		ObjectPtr<ComponentType>	mResource;
		std::string					mPath;
	};


	/**
	* ComponentPtr is used in ComponentInstance classes to point to other ComponentInstance objects directly. ComponentInstances are spawned
	* from Components at runtime. The ComponentInstance class makes sure that the internal pointer is mapped to the other spawned object.
	*
	* The Component of a ComponentInstance must hold a regular ObjectPtr to another Component. when an ComponentPtr is constructed, the user
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
	class ComponentInstancePtr
	{
	public:
		using TargetComponentInstanceType = typename TargetComponentType::InstanceType;

		template<class SourceComponentType>
		ComponentInstancePtr(ComponentInstance* sourceComponentInstance, ComponentPtr<TargetComponentType>(SourceComponentType::*componentMemberPointer))
		{
			SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
			ComponentPtr<TargetComponentType>& target_component_resource = resource->*componentMemberPointer;

			sourceComponentInstance->addToLinkMap(target_component_resource.get(), target_component_resource.getInstancePath(), (ComponentInstance**)&mInstance);
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

		bool operator==(const ComponentInstancePtr<TargetComponentType>& other) const
		{
			return mInstance == other.mPtr;
		}

		template<typename OTHER>
		bool operator==(const ComponentInstancePtr<OTHER>& other) const
		{
			return mInstance == other.mPtr;
		}

		template<typename OTHER>
		bool operator==(const OTHER* ptr) const
		{
			return mInstance == ptr;
		}

		bool operator==(std::nullptr_t) const
		{
			return mInstance == nullptr;
		}

		bool operator!=(const ComponentInstancePtr<TargetComponentType>& other) const
		{
			return mInstance != other.mPtr;
		}

		template<typename OTHER>
		bool operator!=(const ComponentInstancePtr<OTHER>& other) const
		{
			return mInstance != other.mPtr;
		}

		template<typename OTHER>
		bool operator!=(const OTHER* ptr) const
		{
			return mInstance != ptr;
		}

		bool operator!=(std::nullptr_t) const
		{
			return mInstance != nullptr;
		}

		bool operator<(const ComponentInstancePtr<TargetComponentType>& other) const
		{
			return mInstance < other.mInstance;
		}

		bool operator>(const ComponentInstancePtr<TargetComponentType>& other) const
		{
			return mInstance > other.mInstance;
		}

		bool operator<=(const ComponentInstancePtr<TargetComponentType>& other) const
		{
			return mInstance <= other.mInstance;
		}

		bool operator>=(const ComponentInstancePtr<TargetComponentType>& other) const
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

/**
 * The following construct is required to support EntityPtr in RTTR as a regular pointer.
 */
namespace rttr
{
	template<typename T>
	struct wrapper_mapper<nap::ComponentPtr<T>>
	{
		using wrapped_type = T*;
		using type = nap::ComponentPtr<T>;

		inline static wrapped_type get(const type& obj)
		{
			return obj.get();
		}

		inline static type create(const wrapped_type& value)
		{
			return nap::ComponentPtr<T>(value);
		}
	};
}