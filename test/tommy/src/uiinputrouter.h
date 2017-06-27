#pragma once

#include "inputrouter.h"

namespace nap
{
	class CameraComponent;

	/**
	 * Implementation of InputRouter that knows how to route input events to the correct UI element, based on its screen-space position/size
	 */
	class UIInputRouter : public InputRouter
	{
		RTTI_ENABLE(InputRouter)
	public:
		/**
		 * Route the specified input event to the correct InputComponent in the specified list of entities
		 *
		 * @param event The input event
		 * @param entities The entities to consider for routing input to 
		 */
		virtual void routeEvent(const InputEvent& event, const EntityList& entities) override;

		/**
		 * Set the camera used to perform depth sorting
		 * TODO: this should be a property set in json, but we don't currently support Entity pointers
		 */
		void setCamera(CameraComponent& camera) { mCamera = &camera; }

	private:
		CameraComponent* mCamera;	// The camera used for depth sorting
	};
}