/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "component.h"
#include <rtti/typeinfo.h>
#include <rtti/objectptr.h>

namespace nap
{
	/**
	 * This class serves as the base for typed ComponentPtrs
	 * and is only here so that we can check whether a pointer is a ComponentPtr through RTTI.
	 * The reason we can't do that with ComponentPtr itself is because ComponentPtr is a template class
	 * and RTTI has difficulties dealing with that.
	 */
	class ComponentPtrBase
	{
		RTTI_ENABLE();
	public:
        virtual ~ComponentPtrBase() = default;

		/**
		 * Convert the full target path to an ID that can be resolved against an object
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
		 * Assign the target ID & object to this pointer. Used for pointer resolving by the ResourceManager,
		 * should not be called manually (is only public so that we can register it in RTTI)
		 * @param targetID The ID of the target
		 * @param targetObject The pointer to be assigned
		 */
		virtual void assign(const std::string& targetID, rtti::Object& targetObject) = 0;
	};

	/**
	 * Typed version of ComponentPtrBase. ComponentPtr stores the path and the pointer to the target resource.
	 *
	 * When pointing to other other components through ComponentPtrs,
	 * you can do so using a 'path' to the target component. This path can be of two forms:
	 * - A single element, meaning the ComponentPtr is pointing directly to a specific component.
	 * - Multiple elements, separated by a '/',
	 * which means the path points to a specific component located at that path.
	 *
	 * Paths consisting out of multiple elements can be either relative or absolute and come in three forms:
	 * - RootEntityID/ComponentID - An absolute path that starts in the root entity with the specified ID
	 * - ./ChildEntityID/ComponentID - A relative path that starts in the entity that the ComponentPtr is in
	 * - ../ChildEntityID/ComponentID - A relative path starting at the parent of the entity that the ComponentPtr is in
	 *
	 * When there are ChildEntityIDs on the path, it's possible for the ChildEntityID to be ambiguous
	 * (for example, when an entity has multiple children with the same ID).
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
	 * Pointing directly to one of these TransformComponents using a component path is not possible,
	 * because it would be ambiguous.
	 * To disambiguate which specific child entity is meant,
	 * the user can append a ':[child_index]' to the ChildEntityID on the path.
	 *
	 * In this case, to point to the TransformComponent of the second wheel,
	 * the user would use the following path: './WheelEntity:1/TransformComponent'
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

		virtual void assign(const std::string& targetID, rtti::Object& targetObject) override
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
		rtti::ObjectPtr<ComponentType>	mResource;		///< Pointer to the target resource
		std::string						mPath;			///< Path in the entity hierarchy, either relative or absolute
	};

	/**
	 * The ComponentInstancePtrInitProxy is a proxy object that is only used to pass arguments to the correct constructor of ComponentInstancePtr
	 * It's only here so that we can use initComponentInstancePtr to initialize both regular ComponentInstancePtrs, as well as vectors of ComponentInstancePtr.
	 */
	template<typename TargetComponentType, typename SourceComponentType>
	struct ComponentInstancePtrInitProxy
	{
		ComponentInstance*					mSourceComponentInstance;						///< The ComponentInstance that the ComponentInstancePtr is located in
		ComponentPtr<TargetComponentType>	SourceComponentType::*mComponentMemberPointer;	///< Member pointer to the ComponentPtr located in the Component
	};

	/**
	 * ComponentInstancePtr is used in ComponentInstance classes to point to other ComponentInstance objects directly.
	 * ComponentInstances are spawned from Components at runtime. The ComponentInstancePtr class makes sure that
	 * the internal pointer is mapped to the spawned ComponentInstance target object.
	 *
	 * The Component of a ComponentInstance must hold a ComponentPtr to another Component. When an ComponentInstancePtr
	 * is constructed, the user should provide the mapping to the ComponentPtr in the Component, by providing the pointer
	 * to the member.
	 *
	 * Example:
	 *
	 * 		class SomeComponent : public Component
	 *		{
	 *			ComponentPtr<OtherComponent> mOtherComponent;
	 *		};
	 *
	 *		class SomeComponentInstance : public ComponentInstance
	 *		{
	 *			ComponentInstancePtr<OtherComponent> mOtherComponent = initComponentInstancePtr(this, &SomeComponent::mOtherComponent);
	 *		};
	 *
	 * In the example above, SomeComponentInstance::mOtherComponent will point the instance that corresponds to the Component in SomeComponent::mOtherComponent.
	 *
	 * Vectors of ComponentsPtrs are supported in the same way:
	 *
	 * Example:
	 *
	 * 		class SomeComponent : public Component
	 *		{
	 *			std::vector<ComponentPtr<OtherComponent>> mOtherComponentList;
	 *		};
	 *
	 *		class SomeComponentInstance : public ComponentInstance
	 *		{
	 *			std::vector<ComponentInstancePtr<OtherComponent>> mOtherComponentList = initComponentInstancePtr(this, &SomeComponent::mOtherComponentList);
	 *		};
	 *
	 * Here, each element in SomeComponentInstance::mOtherComponentList will point to each instance that corresponds to each element in
	 * the vector SomeComponent::mOtherComponentList;
	 */
	template<class TargetComponentType>
	class ComponentInstancePtr
	{
	public:
		using TargetComponentInstanceType = typename TargetComponentType::InstanceType;

		ComponentInstancePtr() = default;

		/**
		 * Construct a ComponentInstancePtr from a ComponentInstance and member pointer to the ComponentPtr containing the target we're pointing at.
		 * This constructor is deprecated and should not be used anymore; use initComponentInstancePtr instead. It is provided for backwards compatibility only.
		 */
		template<class SourceComponentType>
		ComponentInstancePtr(ComponentInstance* sourceComponentInstance, ComponentPtr<TargetComponentType>(SourceComponentType::* componentMemberPointer))
		{
			SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
			ComponentPtr<TargetComponentType>& target_component_resource = resource->*componentMemberPointer;
			sourceComponentInstance->addToComponentLinkMap(target_component_resource.get(), target_component_resource.getInstancePath(), (ComponentInstance**)&mInstance);
		}

		/**
		* Construct a ComponentInstancePtr from a ComponentInstancePtrInitProxy, which can be retrieved through initComponentInstancePtr.
		*/
		template<class SourceComponentType>
		ComponentInstancePtr(const ComponentInstancePtrInitProxy<TargetComponentType, SourceComponentType>& proxy) :
			ComponentInstancePtr(proxy.mSourceComponentInstance, proxy.mComponentMemberPointer)
		{ }

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

		TargetComponentInstanceType* get() const
		{
			return mInstance;
		}

		TargetComponentInstanceType* get()
		{
			return mInstance;
		}

	private:
		template<typename TargetComponentType_, typename SourceComponentType_>
		friend std::vector<ComponentInstancePtr<TargetComponentType_>> initComponentInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<ComponentPtr<TargetComponentType_>>(SourceComponentType_::*componentMemberPointer));

	private:
		TargetComponentInstanceType* mInstance = nullptr;
	};

	/**
	 * Init a regular ComponentInstancePtr. Returns a ComponentInstancePtrInitProxy which is a simple wrapper around the function arguments.
	 * The return value is passed directly to the corresponding constructor on ComponentInstancePtr, which does the actual initialization.
	 *
	 * @param sourceComponentInstance The ComponentInstance that the ComponentInstancePtr being initialized is a member of
	 * @param componentMemberPointer Member pointer to the ComponentPtr member of the Component
	 *
	 * @return A ComponentInstancePtrInitProxy which can be passed to the ComponentInstancePtr constructor
	 */
	template<typename TargetComponentType, typename SourceComponentType>
	ComponentInstancePtrInitProxy<TargetComponentType, SourceComponentType> initComponentInstancePtr(ComponentInstance* sourceComponentInstance, ComponentPtr<TargetComponentType>(SourceComponentType::*componentMemberPointer));

	/**
	 * Init a std::vector of ComponentInstancePtrs. Returns a std::vector which is then used to initialize the target std::vector of ComponentInstancePtrs
	 *
	 * @param sourceComponentInstance The ComponentInstance that the ComponentInstancePtr being initialized is a member of
	 * @param componentMemberPointer Member pointer to the ComponentPtr member of the Component
	 *
	 * @return std::vector of initialized ComponentInstancePtrs
	 */
	template<typename TargetComponentType, typename SourceComponentType>
	std::vector<ComponentInstancePtr<TargetComponentType>> initComponentInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<ComponentPtr<TargetComponentType>>(SourceComponentType::*componentMemberPointer));
}


//////////////////////////////////////////////////////////////////////////
// The following construct is required to support ComponentPtr in RTTR as a regular pointer.
//////////////////////////////////////////////////////////////////////////
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


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Template definitions
	//////////////////////////////////////////////////////////////////////////

	template<typename TargetComponentType, typename SourceComponentType>
	nap::ComponentInstancePtrInitProxy<TargetComponentType, SourceComponentType>
		initComponentInstancePtr(ComponentInstance* sourceComponentInstance, ComponentPtr<TargetComponentType>(SourceComponentType::*componentMemberPointer))
	{
		return{ sourceComponentInstance, componentMemberPointer };
	}


	template<typename TargetComponentType, typename SourceComponentType>
	std::vector<nap::ComponentInstancePtr<TargetComponentType>>
		initComponentInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<ComponentPtr<TargetComponentType>>(SourceComponentType::*componentMemberPointer))
	{
		SourceComponentType* resource = sourceComponentInstance->getComponent<SourceComponentType>();
		std::vector<ComponentPtr<TargetComponentType>>& target_component_resource = resource->*componentMemberPointer;

		std::vector<ComponentInstancePtr<TargetComponentType>> result;
		result.resize(target_component_resource.size());

		for (int i = 0; i != result.size(); ++i)
			sourceComponentInstance->addToComponentLinkMap(target_component_resource[i].get(), target_component_resource[i].getInstancePath(), (ComponentInstance**)&result[i].mInstance);

		return result;
	}
}
