/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "lineautoswitchcomponent.h"

// External Includes
#include <entity.h>
#include <mathutils.h>
#include <random>
#include <functional>
#include <numeric>

RTTI_BEGIN_CLASS(nap::LineAutoSwitchComponent, "Automatically select a different line when blend operation completes")
	RTTI_PROPERTY("SelectionComponentOne",	&nap::LineAutoSwitchComponent::mSelectionComponentOne,	nap::rtti::EPropertyMetaData::Required, "Link to the first line selector")
	RTTI_PROPERTY("SelectionComponentTwo",	&nap::LineAutoSwitchComponent::mSelectionComponentTwo,	nap::rtti::EPropertyMetaData::Required, "Link to the second line selector")
	RTTI_PROPERTY("BlendComponent",			&nap::LineAutoSwitchComponent::mBlendComponent,			nap::rtti::EPropertyMetaData::Required, "Link to the component that blends between the first and second line selector")
	RTTI_PROPERTY("Random",					&nap::LineAutoSwitchComponent::mRandom,					nap::rtti::EPropertyMetaData::Default,	"Selects a random line when completed instead of the next")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineAutoSwitchComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	void LineAutoSwitchComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
	}


	bool LineAutoSwitchComponentInstance::init(utility::ErrorState& errorState)
	{
		mRandom = getComponent<LineAutoSwitchComponent>()->mRandom;
		mPrevBlendValue = mLineBlender->getCurrentBlendValue();
		return true;
	}


	void LineAutoSwitchComponentInstance::update(double deltaTime)
	{
		// Get current blend value
		float current_blend_value = mLineBlender->getCurrentBlendValue();

		// Compute blend difference
		float current_diff = current_blend_value - mPrevBlendValue;

		// Figure out direction
		EBlendDirection current_direction = EBlendDirection::Stationary;
		if (current_diff > math::epsilon<float>())
		{
			current_direction = EBlendDirection::Up;
		}
		else if (current_diff < (0.0f - math::epsilon<float>()))
		{
			current_direction = EBlendDirection::Down;
		}

		// If no change occurred we can safely return
		if (current_direction == EBlendDirection::Stationary)
			return;

		// Check if we need to set the initial direction, the previous direction is only
		// stationary on initialization
		if (mPrevDirection == EBlendDirection::Stationary)
		{
			mPrevDirection = current_direction;
			return;
		}

		// Calculate average
		if (mBlendDiffs.size() > 4)
			mBlendDiffs.erase(mBlendDiffs.begin());
		mBlendDiffs.emplace_back(math::abs<float>(current_diff));

		// Calculate threshold based on last changed values and give it some slack
		float threshold = (std::accumulate(mBlendDiffs.begin(), mBlendDiffs.end(), 0.0f) / static_cast<float>(mBlendDiffs.size())) * 5.0f;

		// Check if we changed direction and below or under the threshold
		if (mPrevDirection == EBlendDirection::Down && current_direction == EBlendDirection::Up 
			&& current_blend_value < threshold)
		{
			int new_index = mRandom ? mSelectorOne->getIndex() : mNextLine;
			if (mRandom)
			{
				while (new_index == mSelectorOne->getIndex())
				{
					new_index = math::random<int>(0, mSelectorTwo->getCount() - 1);
				}
			}
			std::cout << "switching selector 2: " << new_index << "\n";
			mSelectorTwo->setIndex(new_index);
			mPrevDirection = current_direction;
		}

		else if (mPrevDirection == EBlendDirection::Up && current_direction == EBlendDirection::Down && 
			current_blend_value > (1.0f-threshold))
		{
			int new_index = mRandom ? mSelectorTwo->getIndex() : mNextLine;
			if (mRandom)
			{
				while (new_index == mSelectorTwo->getIndex())
				{
					new_index = math::random<int>(0, mSelectorOne->getCount() - 1);
				}
			}
			std::cout << "switching selector 1: " << new_index << "\n";
			mSelectorOne->setIndex(new_index);
			mPrevDirection = current_direction;
		}
		mPrevBlendValue = current_blend_value;
	}


	void LineAutoSwitchComponentInstance::setLineIndex(int index)
	{
		mNextLine = index;
	}
}
