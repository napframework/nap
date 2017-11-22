#pragma once

#include "entity.h"
#include <nap/service.h>

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

		/**
		 * 
		 */
		bool init(utility::ErrorState& error);

	protected:
		virtual void update(double deltaTime) override;

		/**
		* Recursively updates the transform hierarchy from the root. The entity hierarchy is traversed. For any TransformComponent
		* the world transform is updated.
		*/
		virtual void postUpdate(double deltaTime) override;
	};
}
