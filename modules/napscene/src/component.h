#pragma once

#include "rtti/rttiobject.h"
#include "rtti/objectptr.h"
#include "utility/dllexport.h"

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

	/**
	 * A ComponentInstance is the runtime-instance of a Component, which is read from json.
	 */
	class NAPAPI ComponentInstance : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
        using RTTIObject::init;
        
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
		virtual void update(double deltaTime) {}

		/**
		 * @ return the entity this component belongs to
		 */
		nap::EntityInstance* getEntityInstance() const
		{
			return mEntityInstance;
		}

		/**
		 * @return the resource this component was created from
		 */
		nap::Component* getComponent() const
		{
			return mResource;
		}

		/**
		 * @return the resource this component was created from as type T
		 * This will return a nullptr if the component is not derived from T
		 */
		template<typename T>
		T* getComponent() const;

		/**
		 * Initialize this component from its resource
		 *
		 * @param resource The resource we're being instantiated from
		 * @param entityCreationParams Parameters required to create new entity instances during init
		 * @param errorState The error object
		 */
        virtual bool init(utility::ErrorState& errorState);

	private:
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
	 * A Component is the static data that is deserialized from json. A ComponentInstance is created from it
	 */
	class NAPAPI Component : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)

	public:
		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
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
