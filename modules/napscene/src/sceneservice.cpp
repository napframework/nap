// Local Includes
#include "sceneservice.h"
#include "transformcomponent.h"
#include "scene.h"

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
			updateTransformsRecursive(*child, is_dirty, new_transform);
	}

	//////////////////////////////////////////////////////////////////////////

	bool SceneService::init(utility::ErrorState& error)
	{
		Core& core = getCore();
		core.getResourceManager()->getFactory().addObjectCreator(std::make_unique<SceneCreator>(core));

		return true;
	}

	void SceneService::update(double deltaTime)
	{
		for (Scene* scene : mScenes)
			scene->update(deltaTime);
	}

	void SceneService::postUpdate(double deltaTime)
	{
		for (Scene* scene : mScenes)
			updateTransformsRecursive(scene->getRootEntity(), false, glm::mat4(1.0f));
	}

	void SceneService::registerScene(Scene& scene)
	{
		mScenes.insert(&scene);
	}

	void SceneService::unregisterScene(Scene& scene)
	{
		mScenes.erase(&scene);
	}

}

RTTI_DEFINE(nap::SceneService)
