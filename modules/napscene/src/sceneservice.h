#pragma once

#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	class NAPAPI SceneService : public Service
	{
		RTTI_ENABLE(Service)

	public:
        // Default Constructor
		SceneService() = default;

		// Default Destructor
		virtual ~SceneService() = default;

	protected:
		/**
		* Recursively updates the transform hierarchy from the root. The entity hierarchy is traversed. For any TransformComponent
		* the world transform is updated.
		*/
		virtual void postUpdate(double deltaTime) override;
	};
}
