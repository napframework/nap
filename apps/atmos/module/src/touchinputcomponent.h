#pragma once

#include <component.h>
#include <inputcomponent.h>
#include <nap/resourceptr.h>
#include <parametervec.h>
#include <parameternumeric.h>
#include <smoothdamp.h>

namespace nap
{
	class TouchInputComponentInstance;

	/**
	 *	touchinputcomponent
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
		float mSpeed = 1.0f;							///< Property: 'Speed' movement speed
	};


	/**
	 * touchinputcomponentInstance	
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

		/**
		 * Enable forwarding of events
		 */
		void enable(bool value)							{ mEnabled = value; }

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
		float mSpeed = 1.0f;
		bool mEnabled = true;
	};
}
