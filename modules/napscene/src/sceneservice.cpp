// Local Includes
#include "sceneservice.h"
#include "transformcomponent.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>

namespace nap
{
	void updateTransformsRecursive(EntityInstance& entity, bool parentDirty, const glm::mat4& parentTransform)
	{
		glm::mat4 new_transform = parentTransform;

		bool is_dirty = parentDirty;
		TransformComponentInstance* transform = entity.findComponent<TransformComponentInstance>();
		if (transform && (transform->isDirty() || parentDirty))
		{
			is_dirty = true;
			transform->update(parentTransform);
			new_transform = transform->getGlobalTransform();
		}

		for (EntityInstance* child : entity.getChildren())
		{
			updateTransformsRecursive(*child, is_dirty, new_transform);
		}
	}


	void SceneService::update(EntityInstance& instance)
	{
		updateTransformsRecursive(instance, false, glm::mat4(1.0f));
	}


	void SceneService::postUpdate(double deltaTime)
	{
		updateTransformsRecursive(getCore().getResourceManager()->getRootEntity(), false, glm::mat4(1.0f));
	}
}

RTTI_DEFINE(nap::SceneService)