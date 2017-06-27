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
	/**
	 * Helper function to recursively find all InputComponents that should be considered for input
	 */
	void getInputComponentsRecursive(EntityInstance& entity, std::vector<InputComponent*>& inputComponents)
	{
		// For the UI router, we're only interested in entities that have both a TransformComponent and InputComponent(s)
		if (entity.hasComponent<TransformComponent>())
			entity.getComponentsOfType<InputComponent>(inputComponents);

		for (EntityInstance* child : entity.getChildren())
			getInputComponentsRecursive(*child, inputComponents);
	}


	void nap::UIInputRouter::routeEvent(const InputEvent& event, const EntityList& entities)
	{
		// We're currently only interested in mouse-events
		const PointerEvent* pointer_event = rtti_cast<const PointerEvent>(&event);
		if (pointer_event == nullptr)
			return;

		// Find all input components that should be considered
		std::vector<InputComponent*> input_components;
		for (EntityInstance* entity : entities)
			getInputComponentsRecursive(*entity, input_components);

		// Sort components from front-to-back so that components closer to the camera are at the front of the list
		DepthSorter sorter(DepthSorter::EMode::FrontToBack, mCamera->getViewMatrix());
		std::sort(input_components.begin(), input_components.end(), sorter);

		for (InputComponent* input_component : input_components)
		{
			// Get the world transform. Since we're using an ortographic camera, the world transform is in pixel space.
			TransformComponent& transform_component = input_component->getEntity()->getComponent<TransformComponent>();
			const glm::mat4& world_transform = transform_component.getGlobalTransform();
			
			// Get the size and position of this element.
			// Note: origin of the element is in the center, so we translate by 0.5 * size so we can directly test the mouse coordinates.
			const glm::vec2 size(world_transform[0][0], world_transform[1][1]);
			const glm::vec2 pos(world_transform[3].x - (size.x * 0.5f), world_transform[3].y - (size.y * 0.5f));

			// If the pointer event is in this element, forward the event and break out
			if (pointer_event->mX >= pos.x && pointer_event->mX < pos.x + size.x &&
				pointer_event->mY >= pos.y && pointer_event->mY < pos.y + size.y)
			{
				input_component->trigger(event);
				break;
			}
		}
	}
}

