#pragma once

#include <component.h>
#include <inputcomponent.h>
#include <nap/resourceptr.h>
#include <parametervec.h>
#include <parameternumeric.h>
#include <parametersimple.h>
#include <smoothdamp.h>

namespace nap
{
	class TouchInputComponentInstance;

	/**
	 * Maps touch / mouse input to a vec2 parameter
	 */
	class NAPAPI TouchInputComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TouchInputComponent, TouchInputComponentInstance)
	public:

		/**
		* Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		* @param components the components this object depends on
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ResourcePtr<ParameterVec2>	mParameter;			///< Property: 'Parameter' parameter to map
		ResourcePtr<ParameterBool>	mEnabled;			///< Property: 'Enabled' if this component forwards touch input
		ResourcePtr<ParameterFloat> mSpeed;				///< Property: 'Speed' movement speed
		ResourcePtr<ParameterFloat> mSmoothTime;		///< Property: 'SmoothTime' smooth time
	};


	/**
	 * Maps touch / mouse input to a vec2 parameter
	 */
	class NAPAPI TouchInputComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		TouchInputComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize touchinputcomponentInstance based on the touchinputcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the touchinputcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update touchinputcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

	private:
		/**
		 * Handler for mouse down events
		 */
		void onMouseDown(const PointerPressEvent& pointerPressEvent);

		/**
		 * Handler for mouse move events
		 */
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);

		/**
		 * Handler for mouse up events
		 */
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);

		bool mPressed = false;
		ParameterVec2* mParameter = nullptr;
		ParameterFloat* mSpeed = nullptr;
		ParameterBool* mEnabled = nullptr;
		ParameterFloat* mSmoothTime = nullptr;
		math::Vec2SmoothOperator mSmoother = { {0,0}, 1.0f };
		glm::vec2 mTarget = { 0,0 };
	};
}
