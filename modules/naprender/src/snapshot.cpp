/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "snapshot.h"
#include "snapshotrendertarget.h"
#include "renderablemeshcomponent.h"

#include <nap/logger.h>
#include <nap/numeric.h>
#include <FreeImage.h>
#undef BYTE

constexpr int StitchCombine = 256;	// 0-255 = alpha blended, value > 255 = source image is combined to the distination image

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Snapshot)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width", &nap::Snapshot::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::Snapshot::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Divisor", &nap::Snapshot::mDivisor, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputDirectory", &nap::Snapshot::mOutputDirectory, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ImageFileFormat", &nap::Snapshot::mImageFileFormat, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TextureFormat", &nap::Snapshot::mTextureFormat, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleShading", &nap::Snapshot::mSampleShading, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RequestedSamples", &nap::Snapshot::mRequestedSamples, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor", &nap::Snapshot::mClearColor, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

/**
 * Deduce bitmap type from render texture format. Also informs of the supported output types.
 * @param format the render texture format
 * @return the associated FreeImage type
 */
static FREE_IMAGE_TYPE getFreeImageType(nap::RenderTexture2D::EFormat format)
{
	switch (format)
	{
	case nap::RenderTexture2D::EFormat::R8:
	case nap::RenderTexture2D::EFormat::RGBA8:
	case nap::RenderTexture2D::EFormat::BGRA8:
		return FREE_IMAGE_TYPE::FIT_BITMAP;
	case nap::RenderTexture2D::EFormat::R16:
		return FREE_IMAGE_TYPE::FIT_UINT16;
	case nap::RenderTexture2D::EFormat::RGBA16:
		return FREE_IMAGE_TYPE::FIT_RGBA16;
	}
	return FREE_IMAGE_TYPE::FIT_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Snapshot
	//////////////////////////////////////////////////////////////////////////

	Snapshot::Snapshot(Core& core) :
		mRenderService(core.getService<RenderService>()) {}


	bool Snapshot::init(utility::ErrorState& errorState)
	{
		assert(mWidth > 0 && mHeight > 0);
		mDivisor = mDivisor > 0 ? mDivisor : 1;

		// Ensure the RenderTexture2D format is supported for writing
		if (!errorState.check(getFreeImageType(mTextureFormat) != FREE_IMAGE_TYPE::FIT_UNKNOWN, 
			"%s: Unsupported RenderTexture2D format (%s) for writing to disk", mID.c_str(), rtti::Variant(mTextureFormat).to_string().c_str()))
			return false;
		
		// Verify the output directory. The default is 'data/'
		if (!mOutputDirectory.empty())
		{
			// Check if the configured output directory exists
			std::string absolute_path = utility::getAbsolutePath(mOutputDirectory);
			if (!utility::dirExists(absolute_path))
			{
				// If the directory does not exist, try to create it
				if (!utility::makeDirs(absolute_path))
				{
					errorState.fail("Failed to create directory: %s", absolute_path.c_str());
					return false;
				}
			}
		}

		// Make sure not to create textures that exceed the hardware image dimension limit
		uint32 max_image_dimension = mRenderService->getPhysicalDeviceProperties().limits.maxImageDimension2D;
		if (!errorState.check(mWidth <= max_image_dimension && mHeight <= max_image_dimension, "Snapshot dimension exceeds maximum device image dimension of %d", max_image_dimension))
			return false;

		// Ensure the width and height are divisible by the divisor
		if (!errorState.check(mWidth%mDivisor == 0 && mHeight%mDivisor == 0, "mDivisor (%d) must be a common divisor of mWidth (%d) and mHeight (%d)", mDivisor, mWidth, mHeight))
			return false;

		mCellWidth = mWidth/mDivisor;
		mCellHeight = mHeight/mDivisor;
		mCellSize = { mCellWidth, mCellHeight };

		mNumRows = mDivisor;
		mNumColumns = mDivisor;
		mNumCells = mNumRows * mNumColumns;

		// Inform user in case we have to subdivide the texture
		if (mNumCells > 1)
		{
			Logger::info("%s: Dividing target buffer into %d %dx%d cells", mID.c_str(), mNumRows*mNumColumns, mCellWidth, mCellHeight);
		}

		// Check if we need to render to BGRA for image formats on little endian machines
		// We can ignore this check when rendering 16bit color
		if (mTextureFormat == RenderTexture2D::EFormat::RGBA8 || mTextureFormat == RenderTexture2D::EFormat::BGRA8)
		{
			bool is_little_endian = FI_RGBA_RED == 2;
			mTextureFormat = is_little_endian ? RenderTexture2D::EFormat::BGRA8 : RenderTexture2D::EFormat::RGBA8;
		}

		// Create textures
		mColorTextures.resize(mNumCells);
		for (auto& cell : mColorTextures)
		{
			cell = std::make_unique<RenderTexture2D>(mRenderService->getCore());
			cell->mWidth = mCellWidth;
			cell->mHeight = mCellHeight;
			cell->mFill = true;
			cell->mUsage = ETextureUsage::DynamicRead;
			cell->mFormat = mTextureFormat;

			if (!cell->init(errorState)) 
			{
				errorState.fail("%s: Failed to initialize snapshot cell textures", mID.c_str());
				return false;
			}
		}

		// Keep a record of updated bitmaps
		mCellUpdateFlags.resize(mNumCells, false);

		// Create render target
		mRenderTarget = std::make_unique<SnapshotRenderTarget>(mRenderService->getCore());
		if (!mRenderTarget->init(this, errorState)) 
		{
			errorState.fail("%s: Failed to initialize snapshot rendertarget", mID.c_str());
			return false;
		}

		// Write the destination bitmap when onBitmapsUpdated is triggered
		onCellsUpdated.connect(mSaveBitmapSlot);

		return true;
	}


	void Snapshot::setClearColor(const glm::vec4& color)
	{
		mRenderTarget->setClearColor(color);
	}


	void Snapshot::snap(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps)
	{
		snap(camera, [&comps, &camera, this](nap::SnapshotRenderTarget& target)
		{
			target.beginRendering();
			mRenderService->renderObjects(target, camera, comps);
			target.endRendering();
		});
	}


	void Snapshot::snap(PerspCameraComponentInstance& camera, std::function<void(nap::SnapshotRenderTarget&)> renderCallback)
	{
		camera.setGridDimensions(mNumRows, mNumColumns);
		for (int i = 0; i < mNumCells; i++)
		{
			// Set grid bounds in camera
			uint32 x = i % mNumColumns;
			uint32 y = i / mNumRows;
			camera.setGridLocation(y, x);

			// Select right cell in render target
			mRenderTarget->setCellIndex(i);

			// Render
			renderCallback(*mRenderTarget);
		}
		camera.setGridLocation(0, 0);
		camera.setGridDimensions(1, 1);

		// Create a surface descriptor for the fullsize bitmap
		SurfaceDescriptor fullsize_surface_descriptor = mColorTextures[0]->getDescriptor();
		fullsize_surface_descriptor.mWidth = mWidth;
		fullsize_surface_descriptor.mHeight = mHeight;

		// Allocate the full-size bitmap file buffer if it is empty
		if (!mDestBitmapFileBuffer)
		{
			mDestBitmapFileBuffer = std::make_unique<BitmapFileBuffer>(fullsize_surface_descriptor);
		}

		// Get bitmap storage and type info
		FREE_IMAGE_TYPE fi_image_type = getFreeImageType(mTextureFormat);
		int bits_per_pixel = mColorTextures[0]->getDescriptor().getBytesPerPixel() * 8;

		// Insert callbacks for copying image data per cell from staging buffer directly into the destination bitmap
		for (int i = 0; i < mNumCells; i++)
		{
			mColorTextures[i]->asyncGetData([this, index = i, bpp = bits_per_pixel, type = fi_image_type](const void* data, size_t bytes)
			{
				// Wrap staging buffer data in a bitmap header
				int cell_pitch = mCellWidth*(bpp / 8);
				FIBITMAP* fi_bitmap_src = FreeImage_ConvertFromRawBitsEx(
					false, (uint8*)data, type, mCellWidth, mCellHeight, cell_pitch, bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK
				);

				// Unflatten index
				uint32 x = index % mNumColumns;
				uint32 y = index / mNumRows;

				// Calculate area to copy
				glm::u32vec2 min = { x* mCellWidth, y* mCellHeight };
				glm::u32vec2 max = min + mCellSize;

				// Cast to a FreeImage bitmap handle
				FIBITMAP* fi_bitmap_handle = reinterpret_cast<FIBITMAP*>(mDestBitmapFileBuffer->getHandle());

				// Create view into destination bitmap
				FIBITMAP* fi_bitmap_dst = FreeImage_CreateView(fi_bitmap_handle, min.x, min.y, max.x, max.y);

				// Copy bitmap header src into dest
				FreeImage_Paste(fi_bitmap_dst, fi_bitmap_src, 0, 0, StitchCombine);

				// Unload headers
				FreeImage_Unload(fi_bitmap_src);
				FreeImage_Unload(fi_bitmap_dst);

				// Keep a record of updated bitmaps
				mCellUpdateFlags[index] = true;
				if (mNumCells == 1 || std::find(std::begin(mCellUpdateFlags), std::end(mCellUpdateFlags), false) == std::end(mCellUpdateFlags)) 
				{
					onCellsUpdated();
					mCellUpdateFlags.assign(mNumCells, false);
				}
			});
		}
		onSnapshot();
	}


	bool Snapshot::save()
	{
		// Create a filename for the snapshot file
		std::string path = utility::appendFileExtension(utility::joinPath(
		{
			utility::getAbsolutePath(mOutputDirectory).c_str(),
			timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S_%ms").c_str()
		}), utility::toLower(rtti::Variant(mImageFileFormat).to_string()));

		utility::ErrorState error_state;
		if (!mDestBitmapFileBuffer->save(path, error_state))
		{
			error_state.fail("%s: Failed to save snapshot %s", mID.c_str(), path.c_str());
			nap::Logger::error(error_state.toString());
			return false;
		};

		if (!error_state.hasErrors())
		{
			onSnapshotSaved(path);
		}
		return true;
	}


	void Snapshot::postSnap()
	{

	}
}
