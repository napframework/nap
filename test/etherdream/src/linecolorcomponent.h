#pragma once

// Local Includes
#include "lineblendcomponent.h"

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <glm/glm.hpp>

namespace nap
{
	class LineColorComponentInstance;

	/**
	 *	Resource of the line color component
	 */
	class LineColorComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineColorComponent, LineColorComponentInstance)
	public:
		// property: Color of the line
		glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		// property: link to the component that holds the mesh that we want to color
		ComponentPtr<nap::LineBlendComponent> mBlendComponent;
	};


	class LineColorComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineColorComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

	   /**
		* Initializes this component
		*/
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

	   /**
		* Updates the line color
		*/
		virtual void update(double deltaTime) override;

		/**
		 *	Current color
		 */
		glm::vec4 mColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	private:
		LineBlendComponentInstance* mBlendComponent = nullptr;
	};
}
