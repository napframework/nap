#pragma once

#include "nap/objectptr.h"
#include "component.h"

namespace nap
{
	/**
	 * This class serves as the base for typed ComponentPtrs and is only here so that we can check whether a pointer is a ComponentPtr through RTTI. 
	 * The reason we can't do that with ComponentPtr itself is because ComponentPtr is a template class and RTTI has difficulties dealing with that.
	 */
	class ComponentPtrBase
	{
		RTTI_ENABLE();

	public:
		/**
		 * Convert the full target ID as specified to an ID that can be resolved to an object
		 *
		 * @param targetID The target ID to translate
		 * @return The translated ID
		 */
		static std::string translateTargetID(const std::string& targetID);

		/**
		 * Convert the pointer to a string for serialization
		 * @return The string representation of this object
		 */
		virtual std::string toString() const = 0;

		/**
		 * Assign the target ID & object to this pointer. Used for pointer resolving by the ResourceManager, should not be called manually (is only public so that we can register it in RTTI)
		 * @param targetID The ID of the target
		 * @param targetObject The pointer to be assigned
		 */
		virtual void assign(const std::string& targetID, rtti::RTTIObject& targetObject) = 0;
	};

	/**
	 * Typed version of ComponentPtrBase. ComponentPtr stores the path and the pointer to the target resource. 
	 *
	 * When pointing to other other components through ComponentPtrs, you can do so using a 'path' to the target component. This path can be of two forms:
	 * - A single element, meaning the ComponentPtr is pointing directly to a specific component.
	 * - Multiple elements, separated by a '/', which means the path points to a specific component located at that path.
	 *
	 * Paths consisting out of multiple elements can be either relative or absolute and come in three forms:
	 * - /RootEntityID/ComponentID - An absolute path that starts in the root entity with the specified ID
	 * - ./ChildEntityID/ComponentID - A relative path that starts in the entity that the ComponentPtr is in
	 * - ../ChildEntityID/ComponentID - A relative path that starts in the parent of the entity that the ComponentPtr is in
	 *
	 * When there are ChildEntityIDs on the path, it's possible for the ChildEntityID to be ambiguous (for example, when an entity has multiple children with the same ID).
	 * For example, consider the following entity hierarchy:
	 *
	 * CarEntity
	 *	-> WheelEntity
	 *	    -> TransformComponent
	 *	-> WheelEntity
	 *	    -> TransformComponent
	 *	-> WheelEntity
	 *	    -> TransformComponent
	 *	-> WheelEntity
	 *	    -> TransformComponent
	 *
	 * Pointing directly to one of these TransformComponents using a component path is not possible, because it would be ambiguous.
	 * To disambiguate which specific child entity is meant, the user can append a ':<child_index>' to the ChildEntityID on the path.
	 *
	 * In this case, to point to the TransformComponent of the second wheel, the user would use the following path: './WheelEntity:1/TransformComponent'
	 */
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

		virtual std::string toString() const override
		{
			return mPath;
		}

		virtual void assign(const std::string& targetID, rtti::RTTIObject& targetObject) override
		{
			mPath = targetID;
			mResource = rtti_cast<ComponentType>(&targetObject);
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

		const ComponentType* operator->() const
		{
			assert(mResource != nullptr);
			return mResource.get();
		}

		ComponentType* operator->()
		{
			assert(mResource != nullptr);
			return mResource.get();
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
		ObjectPtr<ComponentType>	mResource;		///< Pointer to the target resource
		std::string					mPath;			///< Path in the entity hierarchy, either relative or absolute
	};


	/**
	* ComponentInstancePtr is used in ComponentInstance classes to point to other ComponentInstance objects directly. ComponentInstances are spawned
	* from Components at runtime. The ComponentInstancePtr class makes sure that the internal pointer is mapped to spawned ComponentInstance target object.
	*
	* The Component of a ComponentInstance must hold a ComponentPtr to another Component. When an ComponentInstancePtr is constructed, the user
	* should provide the mapping to the ComponentPtr in the Component, by providing the pointer to the member. Example:
	* 
	* 		class SomeComponent : public Component
	*		{
	*			ComponentPtr<OtherComponent> mOtherComponent;
	*		};
	*
	*		class SomeComponentInstance : public ComponentInstance
	*		{
	*			ComponentInstancePtr<OtherComponent> mOtherComponent{ this, &SomeComponent::mOtherComponent };
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

			sourceComponentInstance->addToComponentLinkMap(target_component_resource.get(), target_component_resource.getInstancePath(), (ComponentInstance**)&mInstance);
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

	/**
	 * Creates a component resource pointer
	 * @param NAME the variable name of the pointer
	 * @param TYPE the type of the component that it points to
	 */
	#define COMPONENT_POINTER(NAME, TYPE)  ComponentPtr<TYPE> NAME;

	/**
	 * Creates a component instance pointer. This pointer is resolved when creating objects from resources
	 * @param NAME the variable name of the pointer
	 * @param TYPE the type of the component this points to. Needs to be the resource type
	 * @param RESOURCE the component resource that contains the original COMPONENT_POINTER
	 * @param ATTRIBUTE the attribute NAME of the COMPONENT_POINTER
	 */
	#define COMPONENT_INSTANCE_POINTER(NAME, TYPE, RESOURCE, ATTRIBUTE) ComponentInstancePtr<TYPE>	NAME = { this, &RESOURCE::ATTRIBUTE};
}

/**
 * The following construct is required to support ComponentPtr in RTTR as a regular pointer.
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
