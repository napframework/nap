/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderlayer.h"
#include "renderservice.h"

// External Includes
#include <nap/core.h>

RTTI_BEGIN_CLASS(nap::RenderLayer, "Groups together a set of renderable components under a single name so they can be ordered")
	RTTI_PROPERTY("Name", &nap::RenderLayer::mName, nap::rtti::EPropertyMetaData::Default, "Name of the layer")
RTTI_END_STRUCT

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderChain, "Controls the order in which components are rendered")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Layers", &nap::RenderChain::mLayers, nap::rtti::EPropertyMetaData::Default | nap::rtti::EPropertyMetaData::Embedded, "The render layers in ranked order")
RTTI_END_CLASS

namespace nap
{
	RenderChain::RenderChain(nap::Core& core) :
		mRenderService(core.getService<nap::RenderService>()) { }


	bool RenderChain::init(utility::ErrorState& errorState)
	{
		int count = 0;
		mRankMap.reserve(mLayers.size());
		for (const auto& layer : mLayers)
		{
			if (!errorState.check(layer != nullptr, "%s: Empty layer entry encountered", mID.c_str()))
				return false;

			mRankMap[layer.get()] = count;
			++count;
		}

		// Register ourselves
		mRenderService->addChain(*this);
		return true;
	}


	void RenderChain::onDestroy()
	{
		mRenderService->removeChain(*this);
	}
}
