#pragma once
#include "applycolorcomponent.h"

#include <component.h>

namespace nap
{
	class ApplyBBColorComponentInstance;

	/**
	 *	boundscolorcomponent
	 */
	class NAPAPI ApplyBBColorComponent : public ApplyColorComponent
	{
		RTTI_ENABLE(ApplyColorComponent)
		DECLARE_COMPONENT(ApplyBBColorComponent, ApplyBBColorComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
	};


	/**
	 * boundscolorcomponentInstance	
	 */
	class NAPAPI ApplyBBColorComponentInstance : public ApplyColorComponentInstance
	{
		RTTI_ENABLE(ApplyColorComponentInstance)
	public:
		ApplyBBColorComponentInstance(EntityInstance& entity, Component& resource) :
			ApplyColorComponentInstance(entity, resource)									{ }

		/**
		 * Initialize boundscolorcomponentInstance based on the boundscolorcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the boundscolorcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Applies bounding box colors to the mesh
		 */
		virtual void applyColor(double deltaTime) override;
	};
}
