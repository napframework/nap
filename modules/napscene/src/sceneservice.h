#pragma once

#include <nap/service.h>
#include <nap/entityinstance.h>

namespace nap
{
	class SceneService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		SceneService() = default;

		// Default Destructor
		virtual ~SceneService() = default;

		/**
		* Recursively updates the transform hierarchy. The entity hierarchy is traversed. For any TransformComponent
		* the world transform is updated.
		* @param instance: the entity whose transform hierarchy should be updated
		*/
		void update(EntityInstance& instance);

		/**
		* Recursively updates the transform hierarchy from the root. The entity hierarchy is traversed. For any TransformComponent
		* the world transform is updated.
		*/
		void update();
	};
}