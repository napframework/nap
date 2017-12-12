#pragma once

#include "component.h"
#include "mesh.h"

namespace nap
{
	class TransformComponent;
	class ParticleEmitterComponentInstance;
	
	/**
	 * 
	 */
	class ParticleEmitterComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(ParticleEmitterComponent, ParticleEmitterComponentInstance)

		/**
		 * 
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override { components.push_back(RTTI_OF(TransformComponent)); }

	public:
		ObjectPtr<Mesh> mMesh;
	};

	/**
	 * 
	 */
	class ParticleEmitterComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		// Default constructor
		ParticleEmitterComponentInstance(EntityInstance& entity, Component& resource);

		virtual ~ParticleEmitterComponentInstance() override;

		/**
		 * Checks whether a transform component is available.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		
	};
}
