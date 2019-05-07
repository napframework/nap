#pragma once

// External Includes
#include <glm/glm.hpp>
#include <component.h>
#include <componentptr.h>
#include <transformcomponent.h>

namespace nap
{
	class OrbitComponentInstance;

	/**
	* OrbitComponent
	*/
	class NAPAPI OrbitComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrbitComponent, OrbitComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<TransformComponent> mOrbitTransformComponent;		///< Property: 'OrbitTransformComponent' link to the orbit transform component
		ComponentPtr<TransformComponent> mOrbitPathTransformComponent;	///< Property: 'OrbitPathTransformComponent' link to the orbit path transform component
		ComponentPtr<TransformComponent> mOrbitStartTransformComponent;	///< Property: 'OrbitStartTransformComponent' link to the orbit start transform component
		ComponentPtr<TransformComponent> mOrbitEndTransformComponent;	///< Property: 'OrbitEndTransformComponent' link to the orbit end transform component
		ComponentPtr<TransformComponent> mOrbitSunTransformComponent;	///< Property: 'OrbitSunTransformComponent' link to the orbit sun transform component
	};


	/**
	* OrbitComponentInstance
	*/
	class NAPAPI OrbitComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		OrbitComponentInstance(EntityInstance& entity, Component& resource) : ComponentInstance(entity, resource) { }

		/**
		* Initialize OrbitComponentInstance based on the OrbitComponent resource
		* @param errorState should hold the error message when initialization fails
		* @return if the OrbitComponentInstance is initialized successfully
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* update OrbitComponentInstance. This is called by NAP core automatically
		* @param deltaTime time in between frames in seconds
		*/
		virtual void update(double deltaTime) override;

		// Pointers to the run time Component Instances, set during de-serialization
		ComponentInstancePtr<TransformComponent> mOrbitTransformComponent = { this, &OrbitComponent::mOrbitTransformComponent };
		ComponentInstancePtr<TransformComponent> mOrbitPathTransformComponent = { this, &OrbitComponent::mOrbitPathTransformComponent };
		ComponentInstancePtr<TransformComponent> mOrbitStartTransformComponent = { this, &OrbitComponent::mOrbitStartTransformComponent };
		ComponentInstancePtr<TransformComponent> mOrbitEndTransformComponent = { this, &OrbitComponent::mOrbitEndTransformComponent };
		ComponentInstancePtr<TransformComponent> mOrbitSunTransformComponent = { this, &OrbitComponent::mOrbitSunTransformComponent };

		// Update orbit components after properties changed
		void updateOrbit();

		// Exposed properties for GUI
		float		getAngle();
		float		getProgressByTime();

		float		mRadius = 0.874f;
		const float mRadiusMin = 0.5f;
		const float mRadiusMax = 1.5f;

		float		mProgress = 0.0f;

		float		mCenter[2] = { -0.24f, -0.83f };
		const float mCenterRange = 1.5f;

		float		mStartEnd[2] = { 40.0f, 115.0f };
		int			mStartHour = 9;
		int			mEndHour = 18;

		bool		mManualProgress = false;

	private:
		// Initialized Variables
		const glm::vec2		mUvOffset = glm::vec2(6.0f, 2.0f);
		const float			mUvScale = 136.0f;
	};
}
