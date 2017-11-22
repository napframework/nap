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
			scene->updateTransforms(deltaTime);
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
