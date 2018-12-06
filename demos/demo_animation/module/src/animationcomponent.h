#pragma once

#include <fcurve.h>
#include "component.h"
#include "mesh.h"

namespace nap
{
	// Forward declares
	class TransformComponent;
	class AnimatorComponentInstance;
	class ParticleMesh;

	class AnimatorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(AnimatorComponent, AnimatorComponentInstance)

		/**
		 * The particle component needs a transform
		 */
		void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
		}

	public:
		nap::math::FloatFCurve mCurve;
	};


	/**
	 * Runtime particle emitter component
	 */
	class AnimatorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		AnimatorComponentInstance(EntityInstance& entity, Component& resource);

		bool init(utility::ErrorState& errorState) override;

		void update(double deltaTime) override;

	};
}
