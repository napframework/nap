#pragma once

// Local Includes
#include "lineselectioncomponent.h"

// External includes
#include <nap/component.h>
#include <nap/objectptr.h>
#include <nap/componentptr.h>
#include <renderablemeshcomponent.h>
#include <polyline.h>

namespace nap
{
	class LineBlendComponentInstance;

	/**
	 * Resource of the line blend component
	 */
	class LineBlendComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineBlendComponent, LineBlendComponentInstance)
	public:
		// property: the amount to blend between two lines
		float mBlendValue = 0.0f;

		// property: the automatic blend speed
		float mBlendSpeed = 0.0f;

		// property: Link to selection component one
		ComponentPtr<LineSelectionComponent> mSelectionComponentOne;

		// property: Link to selection component two
		ComponentPtr<LineSelectionComponent> mSelectionComponentTwo;

		// property: link to the mesh that we want to blend in between
		ObjectPtr<nap::PolyLine> mTarget;
	};


	/**
	 *	This component blends two lines based on the selection of two other components
	 */
	class LineBlendComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineBlendComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{}

		// Init
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		// Update
		virtual void update(double deltaTime) override;

		/**
		* @return the interpolated line mesh
		*/
		const PolyLine& getLine() const								{ return *mTarget; }

		/**
		 * @return if the blended line is closed or open
		 * The line is considered closed when both source lines are closed
		 */
		bool isClosed() const;

		float mBlendValue = 0.0f;									// Blend value
		float mBlendSpeed = 0.0f;									// Speed to blend between 2 lines

	private:
		PolyLine* mTarget = nullptr;								// Line that will hold the blended values

		// Current time
		float mCurrentTime = 0.0f;

		LineSelectionComponentInstance* mSelectorOne = nullptr;		// First line selection component
		LineSelectionComponentInstance* mSelectorTwo = nullptr;		// Second line selection component

	};
}
