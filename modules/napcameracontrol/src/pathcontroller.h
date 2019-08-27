#pragma once

#include <component.h>
#include <componentptr.h>
#include <polyline.h>
#include <inputevent.h>
#include <transformcomponent.h>
#include <nap/resourceptr.h>
#include <map>

namespace nap
{
	class PathControllerInstance;

	/**
	 * path controller
	 */
	class NAPAPI PathController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(PathController, PathControllerInstance)
	public:

		/**
		 * Get a list of all component types that this component is dependent on (i.e. must be initialized before this one)
		 * @param components the components this object depends on
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		ComponentPtr<TransformComponent> mPathTransform;			///< Transform associated with the path
		ResourcePtr<PolyLine> mPath;								///< Path to place the camera on
		float mPosition;											///< Position of the camera (0 - 1) along the path
		float mMovementSpeed = 0.1;									///< Speed (in seconds) at which the camera is allowed to move
	};


	/**
	 * path controller Instance	
	 */
	class NAPAPI PathControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		PathControllerInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize pathcontrollerInstance based on the path controller resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the pathcontrollerInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update pathcontrollerInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Enable the component while setting the transform.
		 * @param translate Camera translation to set.
		 * @param rotate Camera rotation to set.
		 */
		void enable(float position);

		/**
		* Enable the component, keeping the current transform.
		*/
		void enable();

		/**
		 * Disables this camera controller
		 */
		void disable();

	private:
		/**
		 * Handler for key press events
		 */
		void onKeyPress(const KeyPressEvent& keyPressEvent);

		/**
		 * Handler for key release events
		 */
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);

		ComponentInstancePtr<TransformComponent>mLineTransform = initComponentInstancePtr(this, &PathController::mPathTransform);
		TransformComponentInstance*				mCameraTransform = nullptr;
		bool									mEnabled = true;
		float									mPosition = 0.0f;
		float									mSpeed = 0.0f;
		bool									mMoveForward = false;		// Whether we're moving forward
		bool									mMoveBackward = false;		// Whether we're moving backwards
		std::map<float, int>					mLineDistances;
		PolyLine*								mPath = nullptr;			// Current path
	};
}
