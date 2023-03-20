/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendermask.h"

RTTI_BEGIN_CLASS(nap::RenderTag)
	RTTI_PROPERTY("Name", &nap::RenderTag::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::RenderTagRegistry)
	RTTI_PROPERTY("Tags", &nap::RenderTagRegistry::mTags, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	bool RenderTagRegistry::init(utility::ErrorState& errorState)
	{
		uint count = 0U;
		for (auto& tag : mTags)
		{
			if (!errorState.check(tag != nullptr, "%s: Empty tag entry encountered", mID.c_str()))
				return false;

			tag->mIndex = count;
			++count;
		}
		return true;
	}
}
