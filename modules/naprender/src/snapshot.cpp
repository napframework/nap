/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "snapshot.h"
#include "snapshotrendertarget.h"
#include "renderablemeshcomponent.h"

#include <nap/logger.h>
#include <FreeImage.h>

#undef BYTE
#define STITCH_COMBINE 256

RTTI_BEGIN_ENUM(nap::Snapshot::EOutputExtension)
	RTTI_ENUM_VALUE(nap::Snapshot::EOutputExtension::PNG, "PNG"),
	RTTI_ENUM_VALUE(nap::Snapshot::EOutputExtension::JPG, "JPG"),
	RTTI_ENUM_VALUE(nap::Snapshot::EOutputExtension::TIFF, "TIFF"),
	RTTI_ENUM_VALUE(nap::Snapshot::EOutputExtension::BMP, "BMP")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Snapshot)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width", &nap::Snapshot::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::Snapshot::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaxCellWidth", &nap::Snapshot::mMaxCellWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaxCellHeight", &nap::Snapshot::mMaxCellHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputDir", &nap::Snapshot::mOutputDir, nap::rtti::EPropertyMetaData::FileLink)
	RTTI_PROPERTY("OutputExtension", &nap::Snapshot::mOutputExtension, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Format", &nap::Snapshot::mFormat, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleShading", &nap::Snapshot::mSampleShading, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RequestedSamples", &nap::Snapshot::mRequestedSamples, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor", &nap::Snapshot::mClearColor, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static functions
	//////////////////////////////////////////////////////////////////////////

	static const char* extensionToString(Snapshot::EOutputExtension ext)
	{
		switch (ext) {
		case Snapshot::EOutputExtension::PNG: return "png";
		case Snapshot::EOutputExtension::JPG: return "jpeg";
		case Snapshot::EOutputExtension::TIFF: return "tiff";
		case Snapshot::EOutputExtension::BMP: return "bmp";
		default: return "png";
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Snapshot
	//////////////////////////////////////////////////////////////////////////

	Snapshot::Snapshot(Core& core) :
		mRenderService(core.getService<RenderService>()) {}

	Snapshot::~Snapshot()
	{
		if (mDstBitmapAllocated)
			FreeImage_Unload(mDstBitmap);
	}

	bool Snapshot::init(utility::ErrorState& errorState)
	{
		assert(mWidth > 0 && mHeight > 0);
		assert(mMaxCellWidth > 0 && mMaxCellHeight > 0);

		// Make sure not to create textures that exceed the hardware image dimension limit
		uint32_t max_image_dimension = mRenderService->getPhysicalDeviceProperties().limits.maxImageDimension2D;
		mMaxCellWidth = std::min(mMaxCellWidth, max_image_dimension);
		mMaxCellHeight = std::min(mMaxCellHeight, max_image_dimension);

		// Subdivide into cells if the max cellwidth|cellheight are smaller than the snapshot size
		mNumRows = ceil(mWidth / static_cast<double>(mMaxCellWidth));
		mNumColumns = ceil(mHeight / static_cast<double>(mMaxCellHeight));

		assert(mNumRows > 0 && mNumColumns > 0);
		assert(mNumRows < mWidth / 2 && mNumColumns < mHeight / 2);

		mNumCells = mNumRows * mNumColumns;
		mCellSize = { mWidth / mNumRows, mHeight / mNumColumns };

		// Inform user in case we have to subdivide the texture
		if (mNumCells > 1)
			Logger::info("Snapshot: Dividing target image into %d %dx%d cells", mNumRows*mNumColumns, mCellSize.x, mCellSize.y);

		// Check if we need to render to BGRA for image formats on little endian machines
		// We can ignore this check when rendering 16bit color
		bool is_little_endian = FI_RGBA_RED == 2;
		bool is_8bit_4ch = (mFormat == RenderTexture2D::EFormat::RGBA8 || mFormat == RenderTexture2D::EFormat::BGRA8);
		mFormat = (is_little_endian && is_8bit_4ch) ? RenderTexture2D::EFormat::BGRA8 : RenderTexture2D::EFormat::RGBA8;

		// Create textures
		mColorTextures.resize(mNumCells);
		for (auto& cell : mColorTextures)
		{
			cell = std::make_unique<RenderTexture2D>(mRenderService->getCore());
			cell->mWidth = mCellSize.x;
			cell->mHeight = mCellSize.y;
			cell->mFill = false;
			cell->mUsage = ETextureUsage::DynamicRead;
			cell->mFormat = mFormat;

			if (!cell->init(errorState)) {
				errorState.fail("Failed to initialize snapshot cell textures");
				return false;
			}
		}

		// Get texture storage info
		int bytes_per_pixel = mColorTextures[0]->getDescriptor().getBytesPerPixel();
		uint32_t cell_bytes = mCellSize.x*mCellSize.y*bytes_per_pixel;

		int bpp = bytes_per_pixel * 8;
		int num_channels = mColorTextures[0]->getDescriptor().getNumChannels();

		// Bitmap info
		utility::FIBitmapInfo bitmap_info{};
		if (!utility::getFIBitmapInfo(bpp, num_channels, mWidth, mHeight, bitmap_info)) {
			errorState.fail("Could not generate valid bitmap info");
			return false;
		}
		mFIBitmapInfo = std::make_unique<utility::FIBitmapInfo>(bitmap_info);

		// Keep a record of updated bitmaps
		mBitmapUpdateFlags.resize(mNumCells, false);

		// Create render target
		mRenderTarget = std::make_unique<SnapshotRenderTarget>(mRenderService->getCore());
		if (!mRenderTarget->init(this, errorState)) {
			errorState.fail("%s: Failed to initialize snapshot rendertarget", mID.c_str());
			return false;
		}

		// Inform user if allocation of snapshot buffers was successful
		if (mNumCells > 1)
			Logger::info("Snapshot: Snapshot resource allocation successful");

		// Write the destination bitmap when onBitmapsUpdated is triggered
		onCellsUpdated.connect(writeBitmapSlot);

		return true;
	}

	void Snapshot::setClearColor(const glm::vec4& color)
	{
		mRenderTarget->setClearColor(color);
	}

	bool Snapshot::takeSnapshot(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps)
	{
		// Skip start/stop headless recording calls if commands are being recorded already
		bool skip_recording = mRenderService->getCurrentCommandBuffer() != VK_NULL_HANDLE;

		if (skip_recording || mRenderService->beginHeadlessRecording()) {
			camera.setGridDimensions(mNumRows, mNumColumns);

			for (int i = 0; i < mNumCells; i++) {
				uint32_t x = i % mNumColumns;
				uint32_t y = i / mNumRows;
				camera.setGridLocation(y, x);

				mRenderTarget->setCellIndex(i);
				mRenderTarget->beginRendering();

				mRenderService->renderObjects(*mRenderTarget, camera, comps);
				mRenderTarget->endRendering();
			}
			camera.setGridLocation(0, 0);
			camera.setGridDimensions(1, 1);

			if (!skip_recording)
				mRenderService->endHeadlessRecording();

			// Init destination bitmap (note that color masks are only supported for 16-bit RGBA images and ignored for any other color depth)
			mDstBitmap = FreeImage_AllocateT(mFIBitmapInfo->type, mWidth, mHeight, mFIBitmapInfo->bpp, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
			mDstBitmapAllocated = true;

			// Insert callbacks for copying image data per cell from staging buffer directly into the destination bitmap
			for (int i = 0; i < mNumCells; i++) {
				mColorTextures[i]->asyncGetData([this, i](const void* data, size_t bytes)
				{
					// Wrap staging buffer data in a bitmap header
					int cell_pitch = mCellSize.x*(mFIBitmapInfo->bpp / 8);
					FIBITMAP* fi_bitmap_src = FreeImage_ConvertFromRawBitsEx(
						false, (uint8_t*)data, mFIBitmapInfo->type, mCellSize.x, mCellSize.y, cell_pitch, mFIBitmapInfo->bpp,
						FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK
					);

					// Unflatten index
					uint32_t x = i % mNumColumns;
					uint32_t y = i / mNumRows;

					// Calculate area to copy
					glm::i32vec2 min = glm::i32vec2(x*mCellSize.x, y*mCellSize.y);
					glm::i32vec2 max = min + static_cast<glm::i32vec2>(mCellSize);

					// Create view into destination bitmap
					FIBITMAP* fi_bitmap_dst = FreeImage_CreateView(mDstBitmap, min.x, min.y, max.x, max.y);

					// Copy bitmap header src into dest
					FreeImage_Paste(fi_bitmap_dst, fi_bitmap_src, 0, 0, STITCH_COMBINE);

					// Unload headers
					FreeImage_Unload(fi_bitmap_src);
					FreeImage_Unload(fi_bitmap_dst);

					// Keep a record of updated bitmaps
					mBitmapUpdateFlags[i] = true;

					if (mNumCells == 1 || std::find(std::begin(mBitmapUpdateFlags), std::end(mBitmapUpdateFlags), false) == std::end(mBitmapUpdateFlags)) {
						onCellsUpdated();
						mBitmapUpdateFlags.assign(mNumCells, false);
					}
				});
			}

			onSnapshot();
			return true;
		}
		return false;
	}

	bool Snapshot::writeToDisk()
	{
		std::string output_path = utility::stringFormat("%s/%s.%s", mOutputDir.c_str(), timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S_%ms").c_str(), extensionToString(mOutputExtension));

		utility::ErrorState error_state;
		if (!utility::writeToDisk(mDstBitmap, mFIBitmapInfo->type, output_path, error_state)) {
			nap::Logger::error("error: %s", error_state.toString().c_str());
			return false;
		}

		if (mDstBitmapAllocated) {
			FreeImage_Unload(mDstBitmap);
			mDstBitmapAllocated = false;
		}

		if (!error_state.hasErrors()) {
			onSnapshotSaved(output_path);
		}
		return true;
	}

	std::string Snapshot::getExtension() 
	{ 
		return extensionToString(mOutputExtension); 
	}
}
