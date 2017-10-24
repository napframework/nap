#pragma once

#include <nap/component.h>
#include <smoothdamp.h>
#include <transformcomponent.h>

namespace nap
{
	class XformSmoothComponentInstance;

	/**
	 *	xformsmoothcomponent
	 */
	class XformSmoothComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(XformSmoothComponent, XformSmoothComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		// property: blend speed
		float mBlendSpeed = 0.1f;

		// propery: scale blend speed
		float mScaleBlendSpeed = 0.1f;
	};


	/**
	 * xformsmoothcomponentInstance	
	 */
	class XformSmoothComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		XformSmoothComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize xformsmoothcomponentInstance based on the xformsmoothcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the xformsmoothcomponentInstance is initialized successfully
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 *	Update
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	Set the target value to blend to
		 */
		void setTarget(const glm::vec3& target)		{ mTargetValue = target; }

		/**
		 *	@return the target translate
		 */
		const glm::vec3& getTarget()				{ return mTargetValue; }

		/**
		 *	Set the target scale
		 */
		void setTargetScale(float scale)			{ mTargetScale = scale; }

	private:
		// Transform to update
		TransformComponentInstance* mTransform = nullptr;

		// Smooth operator
		math::SmoothOperator<glm::vec3> mXformSmoother = { { 0.0f, 0.0f, 0.0f }, 0.1f };

		// Smooth scale operator
		math::SmoothOperator<float> mScaleSmoother = { 1.0f, 0.1f };

		// Target xform value
		glm::vec3 mTargetValue = { 0.0,0.0,0.0 };

		// Scale Target Value
		float mTargetScale = 1.0f;
	};
}
