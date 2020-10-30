/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "lineselectioncomponent.h"

// External includes
#include <component.h>
#include <nap/resourceptr.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <polyline.h>
#include <nap/signalslot.h>
#include <parameternumeric.h>

namespace nap
{
	class LineBlendComponentInstance;

	/**
	 * Resource of the line blend component
	 */
	class NAPAPI LineBlendComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(LineBlendComponent, LineBlendComponentInstance)
	public:
		// property: the amount to blend between two lines
		float mBlendValue = 0.0f;

		// property: the automatic blend speed
		ResourcePtr<ParameterFloat> mBlendSpeed;

		// property: Link to selection component one
		ComponentPtr<LineSelectionComponent> mSelectionComponentOne;

		// property: Link to selection component two
		ComponentPtr<LineSelectionComponent> mSelectionComponentTwo;

		// property: link to the mesh that we want to blend in between
		ResourcePtr<nap::PolyLine> mTarget;
	};


	/**
	 * This component blends two lines based on the selection of two other components
	 * This component writes the result to another mesh. Only the position, uv's and normals are blended
	 */
	class NAPAPI LineBlendComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		LineBlendComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{}

		// Init
		virtual bool init(utility::ErrorState& errorState) override;

		// Update
		virtual void update(double deltaTime) override;

		/**
		* @return the interpolated line mesh
		*/
		PolyLine& getLine() const									{ return *mTarget; }

		/**
		 * @return if the blended line is closed or open
		 * The line is considered closed when both source lines are closed
		 */
		bool isClosed() const;

		/**
		 *	@return the current blend value
		 */
		float getCurrentBlendValue() const							{ return mCurrentBlendValue;  }

		/**
		 *	Resets the blend time
		 */
		void reset()												{ mCurrentTime = 0.0f; }

		/**
		 *	@param line the polygon line used as target
		 */
		void setPolyLine(PolyLine& line)							{ mTarget = &line; }

		float mBlendValue = 0.0f;									// Blend value

	private:
		PolyLine* mTarget = nullptr;								// Line that will hold the blended values

		// Current time
		float mCurrentTime = 0.0f;

		// Current blend value
		float mCurrentBlendValue = 0.0f;

		ComponentInstancePtr<LineSelectionComponent> mSelectorOne = { this, &LineBlendComponent::mSelectionComponentOne };
		ComponentInstancePtr<LineSelectionComponent> mSelectorTwo = { this, &LineBlendComponent::mSelectionComponentTwo };

		std::map<float, int>			mDistancesLineOne;			// Distance values associated with line 1
		std::map<float, int>			mDistancesLineTwo;			// Distance values associated with line 2

		std::vector<glm::vec3>			mPositionsLineOne;			// Interpolated positions of the first selected line
		std::vector<glm::vec3>			mPoistionsLineTwo;			// Interpolated positions of the second selected line
		
		std::vector<glm::vec3>			mNormalsLineOne;			// Interpolated Normals associated with the first line
		std::vector<glm::vec3>			mNormalsLineTwo;			// Interpolated Normals associated with the second line
		
		std::vector<glm::vec3>			mUvsLineOne;				// Interpolated Uvs associated with the first line
		std::vector<glm::vec3>			mUVsLineTwo;				// Interpolated Uvs associated with the seconds line

		ParameterFloat*					mBlendSpeed = nullptr;		// Parameter that controls blend speed

		/**
		 * Updates the distance map and re-samples the currently selected curve
		 * This call is necessary for performance reasons, otherwise the getValue<> along line
		 * method is called every update loop, which slows down performance considerably.
		 */
		void cacheVertexAttributes(const LineSelectionComponentInstance& selector);

		/**
		 * Called when the selection of the component changes
		 * Caches the interpolated values for the line currently selected by @selectionComponent
		 */
		void onSelectionChanged(const LineSelectionComponentInstance& selectionComponent);

		// Slot that is called when the index of a selection component changes
		NSLOT(mSelectionChangedSlot, const LineSelectionComponentInstance&, onSelectionChanged);
	};
}