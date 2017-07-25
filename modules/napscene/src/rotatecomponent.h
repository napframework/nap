#pragma once

// External includes
#include <nap/component.h>
#include <nap/dllexport.h>

// Local includes
#include "transformcomponent.h"

#include <glm/glm.hpp>

namespace nap
{
	struct NAPAPI RotateProperties
	{
		glm::vec3	mAxis;		// Rotation axis
		float		mSpeed;		// Rotation speed (seconds)
		float		mOffset;	// Rotation offset in seconds
	};

	//////////////////////////////////////////////////////////////////////////

	class RotateComponentInstance;

	/**
	 * Rotate component resource
	 */
	class NAPAPI RotateComponent : public Component
	{
		RTTI_ENABLE(Component)
	public:
		virtual const rtti::TypeInfo getInstanceType() const override
		{
			return RTTI_OF(RotateComponentInstance);
		}

		/**
		* Uses transform to rotate itself in the world.
		*/
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		RotateProperties mProperties;
	};

	//////////////////////////////////////////////////////////////////////////

	/**
	 * Rotate component instance
	 *
	 * Automatically rotates the entity along a certain axis at a certain speed
	 * The initial rotation value after initialization is used to rotate along the specified axis
	 * This component updates (overrides) the rotate value of the transform component
	 * Entities that use this component must have a transform
	 */
	class NAPAPI RotateComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		RotateComponentInstance(EntityInstance& entity) : ComponentInstance(entity)		{ }

		/**
		 * Initialize this rotate component, copies it's members over and validates
		 * if a transform component is available that is rotated
		 * @param resource the resource we're instantiated from
		 * @param used for creating new entity instances
		 * @param errorState the error object
		 */
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Rotates the component every tick based on the speed and exis
		 * @param deltaTime time it took to complete last cook (seconds)
		 */
		virtual void update(double deltaTime) override;

		// Rotation properties
		RotateProperties mProperties;

	private:
		// Store pointer to transform, set during init
		nap::TransformComponentInstance* mTransform;

		// Local elapsed time
		double mElapsedTime = 0.0;

		// Initial Rotation value
		glm::quat mInitialRotate = glm::quat();
	};
}