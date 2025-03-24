/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "uvtesttexture.h"
#include "bitmap.h"

// External includes
#include <nap/core.h>
#include <renderservice.h>
#include <mutex>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UVTestTexture, "Checkerboard test texture to inspect uv coordinates")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("GenerateLods", &nap::UVTestTexture::mGenerateLods, nap::rtti::EPropertyMetaData::Default,
		"If lower levels of detail (LODs) are auto-generated for the image")
RTTI_END_CLASS

namespace nap
{
	/**
	 * Attempts to load and return the uv test bitmap.
	 * @param module the module to load the asset from
	 * @param error contains the error if loading fails
	 * @return uv test bitmap
	 */
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


	UVTestTexture::UVTestTexture(Core& core) : Texture2D(core)
	{ }


	bool UVTestTexture::init(utility::ErrorState& errorState)
	{
		// Get bitmap instance
		auto* bitmap = getBitmap(mRenderService.getModule(), errorState);
		if (!errorState.check(bitmap != nullptr, "Unable to load bitmap"))
			return false;

		// Ensure hardware mip-mapping is supported
		if (!errorState.check(!mGenerateLods || mRenderService.getMipSupport(bitmap->mSurfaceDescriptor),
			"%s: hardware mipmap generation not supported, consider disabling 'GenerateLods'", mID.c_str()))
			return false;

		// Initialize texture on GPU
		int lvl = mGenerateLods ? utility::computeMipLevel(bitmap->mSurfaceDescriptor) : 1;
		return Texture2D::init(bitmap->getDescriptor(), EUsage::Static,
			lvl, bitmap->getData(), 0, errorState);
	}
}
