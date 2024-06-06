/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "lineselectioncomponent.h"

// External Includes
#include <mathutils.h>
#include <entity.h>
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::LineSelectionComponent)
	RTTI_PROPERTY("Lines", &nap::LineSelectionComponent::mLines, nap::rtti::EPropertyMetaData::Required, "List of available poly lines")
	RTTI_PROPERTY("Index", &nap::LineSelectionComponent::mIndex, nap::rtti::EPropertyMetaData::Required, "Initial line selection index")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::LineSelectionComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	bool LineSelectionComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy over the list of lines to selection from
		mLines = getComponent<LineSelectionComponent>()->mLines;

		// Ensure there are lines to choose from
		if (!(errorState.check(mLines.size() > 0, "No lines to select from")))
			return false;

		// Make sure index is in range
		verifyIndex(getComponent<LineSelectionComponent>()->mIndex);

		return true;
	}


	const nap::PolyLine& LineSelectionComponentInstance::getLine() const
	{
		return *(mLines[mIndex]);
	}


	nap::PolyLine& LineSelectionComponentInstance::getLine()
	{
		return *(mLines[mIndex]);
	}


	void LineSelectionComponentInstance::setIndex(int index)
	{
		verifyIndex(index);
	}


	void LineSelectionComponentInstance::verifyIndex(int index)
	{
		// Make sure the index is in range
		int new_index = math::clamp<int>(index, 0, mLines.size() - 1);
		if (new_index == mIndex)
			return;

		// Set and trigger
		mIndex = new_index;
		mIndexChanged.trigger(*this);
	}
}
