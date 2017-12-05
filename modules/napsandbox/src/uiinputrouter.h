#pragma once

#include "inputrouter.h"
#include "orthocameracomponent.h"
#include "cameracomponent.h"
#include <component.h>
#include <componentptr.h>

namespace nap
{
	class UIInputRouterComponentInstance;

	/**
	 * Implementation of InputRouter that knows how to route input events to the correct UI element, based on its screen-space position/size
	 */
	class NAPAPI UIInputRouter : public InputRouter
	{
	public:
		/**
		 * Set the camera used to perform depth sorting
		 */
		bool init(CameraComponentInstance& camera, utility::ErrorState& errorState) { mCamera = &camera; return true; }

		/**
		 * Route the specified input event to the correct InputComponent in the specified list of entities
		 *
		 * @param event The input event
		 * @param entities The entities to consider for routing input to 
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities) override;

	private:
		CameraComponentInstance* mCamera;	// The camera used for depth sorting
	};


	/**
	 * This component holds a pointer to the camera entity so that the instance can 
	 * access the camera entity to initialize it's camera.
	 */
	class NAPAPI UIInputRouterComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(UIInputRouterComponent, UIInputRouterComponentInstance)

	public:
		ComponentPtr<OrthoCameraComponent> mCameraComponent;		// Pointer to camera entity resource
	};

	/**
	 * Wrapper for a UIInputRouter. Retrieves the camera entity and it's camera component to initialize
	 * the input router.
	 */
	class NAPAPI UIInputRouterComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)

	public:
		/**
		 * Retrieves the camera entity and it's camera component to initialize the input router.
		 * @param resource UIInputRouterComponentResource
		 * @return true on success, other false.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		UIInputRouterComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{
		}
		
		ComponentInstancePtr<OrthoCameraComponent> mCameraComponent { this, &UIInputRouterComponent::mCameraComponent };
		UIInputRouter mInputRouter;		// UI input router
	};
}