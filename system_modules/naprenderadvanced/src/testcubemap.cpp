/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "testcubemap.h"

// External includes
#include <nap/core.h>
#include <bitmap.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TestCubeMap, "A basic face labeled cubemap for testing purposes")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SunsetCubeMap, "A basic sunset cubemap for testing purposes")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
	/**
	 * Loads an equirectangular bitmap from path
	 * @param path path pointing to bitmap
	 * @param module module that holds the asset
	 * @param error the error when loading fails
	 */
	static std::unique_ptr<BitmapFromFile> loadBitmap(const char* path, const nap::Module& module, nap::utility::ErrorState& error)
	{
		// Find texture asset
		auto asset_path = module.findAsset(path);
		if (!error.check(!asset_path.empty(), "Unable to locate: %s", asset_path))
			return nullptr;

		// Create and load the bitmap
		auto bitmap = std::make_unique<BitmapFromFile>();
		bitmap->mPath = std::move(asset_path);
		if (!bitmap->init(error))
			bitmap.reset(nullptr);

		return bitmap;
	}


	TestCubeMap::TestCubeMap(Core& core) : EquiRectangularCubeMap(core),
		mEquiRectangularTexture(core)
	{ }


	bool TestCubeMap::init(utility::ErrorState& errorState)
	{
		// load test bitmap only once
		static std::unique_ptr<BitmapFromFile> test_bitmap = nullptr;
		static std::once_flag flag;
		std::call_once(flag, [this, &errorState]()
			{
				static constexpr const char* cube_tex = "textures/cube_equirectangular_face_test.jpg";
				test_bitmap = loadBitmap(cube_tex, mRenderAdvancedService->getModule(), errorState);
			});

		// If test bitmap isn't available, bail
		if (!errorState.check(test_bitmap != nullptr, "Unable to load equirectangular bitmap"))
			return false;

		// Initialize equirectangular texture on GPU
		if (!mEquiRectangularTexture.init(test_bitmap->getDescriptor(), Texture2D::EUsage::Static, 1,
			test_bitmap->getData(), 0, errorState))
			return false;

		// Generate cubemap
		return EquiRectangularCubeMap::init(mEquiRectangularTexture, errorState);
	}


	SunsetCubeMap::SunsetCubeMap(Core& core) : EquiRectangularCubeMap(core),
		mEquiRectangularTexture(core)
	{ }


	bool SunsetCubeMap::init(utility::ErrorState& errorState)
	{
		// Load sunset bitmap only once
		static std::unique_ptr<BitmapFromFile> sunset_bitmap = nullptr;
		static std::once_flag sunset_flag;
		std::call_once(sunset_flag, [this, &errorState]()
			{
				// Find texture asset
				static constexpr const char* cube_tex = "textures/cube_equirectangular_sunset.jpg";
				sunset_bitmap = loadBitmap(cube_tex, mRenderAdvancedService->getModule(), errorState);
			});

		// If sunset bitmap isn't available, bail
		if (sunset_bitmap == nullptr)
			return false;

		// Initialize equirectangular texture on GPU
		if (!mEquiRectangularTexture.init(sunset_bitmap->getDescriptor(), Texture2D::EUsage::Static, 1,
			sunset_bitmap->getData(), 0, errorState))
			return false;

		// Generate cubemap
		return EquiRectangularCubeMap::init(mEquiRectangularTexture, errorState);
	}
}

