#pragma once

#include "napinputservice.h"

namespace nap
{
	class CameraComponent;

	class UIInputRouter : public InputRouter
	{
		RTTI_ENABLE(InputRouter)
	public:
		virtual void routeEvent(const InputEvent& event, const EntityList& entities) override;

		void setCamera(CameraComponent& camera) { mCamera = &camera; }

	private:
		CameraComponent* mCamera;
	};
}