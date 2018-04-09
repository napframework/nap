// Local Includes
#include "sceneservice.h"
#include "transformcomponent.h"
#include "scene.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/resourcemanager.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SceneService)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	SceneService::SceneService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}

	void SceneService::registerObjectCreators(rtti::Factory& factory)
	{
		factory.addObjectCreator(std::make_unique<SceneCreator>(getCore()));
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

