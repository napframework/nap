#pragma once

#include "rtti/rttiobject.h"
#include "nap/objectptr.h"
#include "utility/dllexport.h"

namespace nap
{
	// Forward Declares
	namespace utility
	{
		class ErrorState;
	}

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
        virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

	private:
		template<class TargetComponentType> friend class ComponentInstancePtr;
		friend class ResourceManager;
		
		/**
		 * Called by ComponentInstancePtr on construction. Adds the ComponentPtrInstance to the internal link map. The link map is
		 * used later by the ResourceManager for resolve pointers to component instances.
		 * @param targetResource The component resource that is being pointed to.
		 * @param instancePath The entity path in the hierarchy that is being pointed to.
		 * @param targetInstancePtr The address of the pointer that needs to be filled in during resolve.
		 */
		void addToLinkMap(Component* targetResource, const std::string& instancePath, ComponentInstance** targetInstancePtr);

	private:
		/**
		 * Holds information needed to resolve component instance pointers.
		 */
		struct TargetComponentLink
		{
			ComponentInstance**		mTargetPtr;		///< The address of the pointer that needs to be filled in during resolve.
			std::string				mInstancePath;	///< The entity path in the hierarchy that is being pointed to.
		};

		using LinkMap = std::unordered_map<Component*, std::vector<TargetComponentLink>>;
		LinkMap			mLinkMap;			// Map containing component instance link information that is used to resolve pointers
		EntityInstance* mEntityInstance;	// The entity this component belongs to
		Component*		mResource;			// The resource this instance was created from
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
