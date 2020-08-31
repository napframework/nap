#pragma once

// Local Includes
#include "lineblendcomponent.h"
#include "lineselectioncomponent.h"

// External Includes
#include <componentptr.h>
#include <component.h>

namespace nap
{
	class LineAutoSwitchComponentInstance;

	/**
	 *	Resource for component that automatically switches the line selection
	 */
	class NAPAPI LineAutoSwitchComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineAutoSwitchComponent, LineAutoSwitchComponentInstance)
	public:
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;
		
		// Property: first selection component
		ComponentPtr<LineSelectionComponent> mSelectionComponentOne;

		// Property: second selection component
		ComponentPtr<LineSelectionComponent> mSelectionComponentTwo;

		// Property: blend component
		ComponentPtr<LineBlendComponent> mBlendComponent;

		// Property: if the line switching is random
		bool mRandom = false;
	};


	/**
	 * Automatically updates the line selection of a lineselectioncomponent,
	 * based on the blend value of the LineBlendComponent. Switching can occur
	 * randomly or based on an explicit next line index
	 */
	class NAPAPI LineAutoSwitchComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineAutoSwitchComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource) {}

		/**
		 *	Initializes the switcher
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	Checks if the blend threshold is close to 0 or 1 and switches lines accordingly
		 */
		virtual void update(double deltaTime) override;

		/**
		 * @param index the index of the line to blend to next (starting at 0)
		 */
		void setLineIndex(int index);

		/**
		 *	@param if next line selection is chosen at random instead of index
		 */
		void setRandom(bool value)									{ mRandom = value;  }

	private:
		/**
		 *	Used internally to figure and diff blend direction
		 */
		enum class EBlendDirection : int
		{
			Up			= 0,
			Down		= 1,
			Stationary	= 2
		};

		ComponentInstancePtr<LineSelectionComponent> mSelectorOne		= { this, &LineAutoSwitchComponent::mSelectionComponentOne };		// First line selection component
		ComponentInstancePtr<LineSelectionComponent> mSelectorTwo		= { this, &LineAutoSwitchComponent::mSelectionComponentTwo };		// Second line selection component
		ComponentInstancePtr<LineBlendComponent>	mLineBlender		= { this, &LineAutoSwitchComponent::mBlendComponent };				// Line Blender
		bool mRandom = false;											// If this component randomly picks another line
		int mNextLine = 0;												// The next line to select (if random is turned off)

		float mPrevBlendValue = 0.0f;									// Previous blend value
		EBlendDirection mPrevDirection = EBlendDirection::Stationary;	// Current blend direction
		std::vector<float> mBlendDiffs;									// Holds the blend differences
	};
}