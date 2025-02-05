/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "uvgridtexture.h"
#include "bitmap.h"

// External includes
#include <threads.h>
#include <nap/core.h>
#include <renderservice.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UVGridTexture)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("GenerateLods", &nap::UVGridTexture::mGenerateLods, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	static nap::Bitmap* getBitmap(const nap::Module& module, nap::utility::ErrorState& error)
	{
		static constexpr const char* uvGrid = "textures/uv_grid_texture.jpg";
		static std::unique_ptr<nap::BitmapFromFile> bitmap = nullptr;
		static std::once_flag flag;
		std::call_once(flag, [&]()
			{
				// Find texture asset
				auto asset_path = module.findAsset(uvGrid);
				if (!error.check(!asset_path.empty(), "Unable to locate: %s", uvGrid))
					return;

				// Create and init the bitmap
				bitmap = std::make_unique<BitmapFromFile>();
				bitmap->mPath = std::move(asset_path);
				if (!bitmap->init(error))
					bitmap.reset(nullptr);
			});
		return bitmap.get();
	}


	UVGridTexture::UVGridTexture(Core& core) : Texture2D(core)
	{ }


	bool UVGridTexture::init(utility::ErrorState& errorState)
	{
		// Get bitmap instance
		auto* bitmap = getBitmap(mRenderService.getModule(), errorState);
		if (bitmap == nullptr)
			return false;

		// Initialize texture on GPU
		mUsage = EUsage::Static;
		return Texture2D::init(bitmap->getDescriptor(),
			mGenerateLods, bitmap->getData(), 0, errorState);
	}
}
