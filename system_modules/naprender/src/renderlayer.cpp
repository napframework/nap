/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "RenderLayer.h"

RTTI_BEGIN_CLASS(nap::RenderLayer)
	RTTI_PROPERTY("Name", &nap::RenderLayer::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_STRUCT

RTTI_BEGIN_CLASS(nap::RenderLayerRegistry)
	RTTI_PROPERTY("Layers", &nap::RenderLayerRegistry::mLayers, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
	bool RenderLayerRegistry::init(utility::ErrorState& errorState)
	{
		uint count = 0U;
		for (auto& layer : mLayers)
		{
			if (!errorState.check(layer != nullptr, "%s: Empty layer entry encountered", mID.c_str()))
				return false;

			layer->mIndex = count;
			++count;
		}
		return true;
	}
}
