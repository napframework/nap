#pragma once

#include <component.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <parameterquat.h>

namespace nap
{
	class UpdateTransformComponentInstance;
	class TransformComponentInstance;

	/**
	 *	UpdateTransformComponent
	 */
	class NAPAPI UpdateTransformComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UpdateTransformComponent, UpdateTransformComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterVec3>				mPosition;
		ResourcePtr<ParameterQuat>				mOrientation;
		ResourcePtr<ParameterFloat>				mUniformScale;
	};


	/**
	 * UpdateTransformComponentInstance	
	 */
	class NAPAPI UpdateTransformComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		UpdateTransformComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize UpdateTransformComponentInstance based on the UpdateTransformComponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the UpdateTransformComponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update UpdateTransformComponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		UpdateTransformComponent* mResource = nullptr;
		TransformComponentInstance* mTransformComponent = nullptr;
	};
}
