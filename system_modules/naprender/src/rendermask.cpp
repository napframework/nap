/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "rendermask.h"
#include "renderservice.h"

// External includes
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTag)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Name", &nap::RenderTag::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

DEFINE_GROUP(nap::RenderTagGroup)

namespace nap
{
	RenderTag::RenderTag(Core& core) :
		mRenderService(*core.getService<RenderService>())
	{ }


	bool RenderTag::start(utility::ErrorState& errorState)
	{
		return mRenderService.addTag(*this, errorState);
	}


	void RenderTag::stop()
	{
		mRenderService.removeTag(*this);
	}


	nap::RenderMask RenderTag::getMask() const
	{
		return 1 << mRenderService.getTagIndex(*this);
	}
}
