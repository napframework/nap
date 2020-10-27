/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/object.h>
#include <rtti/objectptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>

#include <cassert>

namespace nap
{
	// Forward Declares
	namespace utility
	{
		class ErrorState;
	}

	class Entity;
	class EntityInstance;
	class Component;
	struct EntityCreationParameters;

	template<class TargetComponentType>
	class ComponentInstancePtr;

	template<class ComponentType>
	class ComponentPtr;

	class EntityPtr;
	class EntityInstancePtr;

	/**
	 * Runtime version of a Component.
	 * Adds behavior to an entity and allows for operations on a per frame basis.
	 * Override the init and update methods in derived classes
	 * Every runtime version of a component receives on construction the resource it was created from and
	 * the entity instance it belongs to.
	 */
	class NAPAPI ComponentInstance : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)

	public:
        using Object::init;
        
		/**
		 * Constructor
		 * @param entity instance the entity this component instance belongs to
		 * @param resource the resource this component was created from
		 */
		ComponentInstance(EntityInstance& entity, Component& resource) : 
			mEntityInstance(&entity),
			mResource(&resource)
		{ 
		}

		/**
		 * Update this component
		 * @param deltaTime the time in between cooks in seconds
		 */
		virtual void update(double deltaTime)				{ }

		/**
		 * @return the entity this component belongs to
		 */
		nap::EntityInstance* getEntityInstance() const 		{ return mEntityInstance; }

		/**
		 * @return the resource this component was created from
		 */
		nap::Component* getComponent() const				{ return mResource; }

		/**
		 * @return the resource this component was created from as type T
		 * This will return a nullptr if the component is not derived from T
		 */
		template<typename T>
		T* getComponent() const;

		/**
		 * Initializes this component based on it's resource.
		 * @param errorState contains the error when initialization fails.
		 * @return if initialization succeeded.
		 */
        virtual bool init(utility::ErrorState& errorState);

	private:
		template<typename TargetComponentType, typename SourceComponentType>
		friend std::vector<ComponentInstancePtr<TargetComponentType>> initComponentInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<ComponentPtr<TargetComponentType>>(SourceComponentType::*componentMemberPointer));

		template<typename SourceComponentType>
		friend std::vector<EntityInstancePtr> initEntityInstancePtr(ComponentInstance* sourceComponentInstance, std::vector<EntityPtr>(SourceComponentType::*entityMemberPointer));

		template<class TargetComponentType> friend class ComponentInstancePtr;
		friend class EntityInstancePtr;
		friend class SceneInstantiation;
		
		/**
		 * Called by ComponentInstancePtr on construction. Adds the ComponentPtrInstance to the internal link map. The link map is
		 * used later by the Scene for resolve pointers to component instances.
		 * @param targetResource The component resource that is being pointed to.
		 * @param instancePath The entity path in the hierarchy that is being pointed to.
		 * @param targetInstancePtr The address of the pointer that needs to be filled in during resolve.
		 */
		void addToComponentLinkMap(Component* targetResource, const std::string& instancePath, ComponentInstance** targetInstancePtr);

		/**
		* Called by EntityInstancePtr on construction. Adds the EntityPtrInstance to the internal link map. The link map is
		* used later by the Scene for resolve pointers to entity instances.
		* @param targetResource The entity resource that is being pointed to.
		* @param instancePath The entity path in the hierarchy that is being pointed to.
		* @param targetInstancePtr The address of the pointer that needs to be filled in during resolve.
		*/
		void addToEntityLinkMap(Entity* targetResource, const std::string& instancePath, EntityInstance** targetInstancePtr);

	private:
		/**
		 * Holds information needed to resolve instance pointers.
		 */
		template<class INSTANCETYPE>
		struct TargetInstanceLink
		{
			INSTANCETYPE**		mTargetPtr;		///< The address of the pointer that needs to be filled in during resolve.
			std::string			mInstancePath;	///< The entity path in the hierarchy that is being pointed to.
		};

		using TargetComponentLink = TargetInstanceLink<ComponentInstance>;
		using TargetEntityLink = TargetInstanceLink<EntityInstance>;

		using ComponentLinkMap	= std::unordered_map<Component*, std::vector<TargetComponentLink>>;
		using EntityLinkMap		= std::unordered_map<Entity*, std::vector<TargetEntityLink>>;

		ComponentLinkMap	mComponentLinkMap;	// Map containing component instance link information that is used to resolve pointers
		EntityLinkMap		mEntityLinkMap;		// Map containing entity instance link information that is used to resolve pointers
		EntityInstance*		mEntityInstance;	// The entity this component belongs to
		Component*			mResource;			// The resource this instance was created from
	};

	///////////////////////////////////////////////////////////////////////////
	// Component
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Resource part of a component. This represents the the static data that is deserialized from json. 
	 * A run time counterpart is created after deserialization. Derived classes need to implement the DECLARE_COMPONENT macro,
	 * This macro tells the system what the run time counterpart of the resource is. 
	 */
	class NAPAPI Component : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		/**
		 * Populates a list of components this component depends on.
		 * Every component dependency, when found, is initialized before this component.
		 * A dependency is NOT a hard requirement. Serialization will not fail if the dependent component 
		 * isn't declared in JSON. It only means that when a component is declared under the same entity,
		 * and that component is tagged as a dependency of this component, it is initialized before this component.
		 * It is your responsibility to return false on initialization if the dependency is a hard requirement 
		 * and can't be found. To ensure the right order of initialization based on a hard requirement it
		 * is advised to use a nap::ComponentPtr instead.
		 * @param components list of component types this resource depends on.
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const { }

		/** 
		 * Get the type of ComponentInstance that should be created from this Component
		 */
		virtual const rtti::TypeInfo getInstanceType() const = 0;
	
	private:
		/**
		 * @return If this Component was cloned from another Component (for instance properties), this returns the ID of the Component it was cloned from. Otherwise, just the regular ID.
		 */
		const std::string& getOriginalID() const;

	private:
		friend class Scene;
		friend class SceneInstantiation;
		Component* mOriginalComponent = nullptr;	// If this Component was cloned from another component (for instance properties), this property holds the Component it was cloned from
	};

	//////////////////////////////////////////////////////////////////////////

	template<typename T>
	T* ComponentInstance::getComponent() const
	{
		T* comp = rtti_cast<T>(mResource);
		assert(comp != nullptr);
		return comp;
	}


#define DECLARE_COMPONENT(ComponentType, ComponentInstanceType)				\
	public:																	\
		using InstanceType = ComponentInstanceType;							\
		virtual const rtti::TypeInfo getInstanceType() const override		\
		{																	\
			return nap::rtti::TypeInfo::get<InstanceType>();				\
		}																	\
	private:

}
