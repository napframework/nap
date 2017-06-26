#include "uiinputrouter.h"
#include "nap/entity.h"
#include "transformcomponent.h"
#include "napinputcomponent.h"
#include "cameracomponent.h"
#include "depthsorter.h"

RTTI_BEGIN_CLASS(nap::UIInputRouter)
RTTI_END_CLASS

namespace nap
{
	void getUIEntitiesRecursive(EntityInstance& entity, std::vector<InputComponent*>& inputComponents)
	{
		if (entity.hasComponent<TransformComponent>())
			entity.getComponentsOfType<InputComponent>(inputComponents);

		for (EntityInstance* child : entity.getChildren())
			getUIEntitiesRecursive(*child, inputComponents);
	}

	void nap::UIInputRouter::routeEvent(const InputEvent& event, const EntityList& entities)
	{
		const PointerEvent* pointer_event = rtti_cast<const PointerEvent>(&event);
		if (pointer_event == nullptr)
			return;

		std::vector<InputComponent*> input_components;
		for (EntityInstance* entity : entities)
			getUIEntitiesRecursive(*entity, input_components);

		DepthSorter sorter(DepthSorter::EMode::FrontToBack, mCamera->getViewMatrix());
		std::sort(input_components.begin(), input_components.end(), sorter);

		for (InputComponent* input_component : input_components)
		{
			TransformComponent& transform = input_component->getEntity()->getComponent<TransformComponent>();
			const glm::mat4& world_transform = transform.getGlobalTransform();
			const glm::vec2 size(world_transform[0][0], world_transform[1][1]);
			const glm::vec2 pos(world_transform[3].x - (size.x * 0.5f), world_transform[3].y - (size.y * 0.5f));

			if (pointer_event->mX >= pos.x && pointer_event->mX < pos.x + size.x &&
				pointer_event->mY >= pos.y && pointer_event->mY < pos.y + size.y)
			{
				input_component->trigger(event);
				break;
			}
		}
	}
}

