/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "snapshot.h"
#include "bitmaputils.h"

#include <nap/logger.h>
#include <renderablemeshcomponent.h>

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
	RTTI_PROPERTY("NumRows", &nap::Snapshot::mNumRows, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("NumColumns", &nap::Snapshot::mNumColumns, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OutputDir", &nap::Snapshot::mOutputDir, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OutputExtension", &nap::Snapshot::mOutputExtension, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Format", &nap::Snapshot::mFormat, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("SampleShading", &nap::Snapshot::mSampleShading, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("RequestedSamples", &nap::Snapshot::mRequestedSamples, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ClearColor", &nap::Snapshot::mClearColor, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Stitch", &nap::Snapshot::mStitch, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	Snapshot::Snapshot(Core& core) :
		mRenderService(core.getService<RenderService>()) {}

	Snapshot::~Snapshot() {}

	const char* extensionToString(Snapshot::EOutputExtension ext)
	{
		switch (ext) {
		case Snapshot::EOutputExtension::PNG: return "png";
		case Snapshot::EOutputExtension::JPG: return "jpeg";
		case Snapshot::EOutputExtension::TIFF: return "tiff";
		case Snapshot::EOutputExtension::BMP: return "bmp";
		default: return "png";
		}
	}

	bool Snapshot::init(utility::ErrorState& errorState)
	{
		assert(mNumRows > 0 && mNumColumns > 0);
		assert(mWidth > mNumRows && mHeight > mNumColumns);

		nap::ResourceManager* resourceManager = mRenderService->getCore().getResourceManager();

		uint32_t max_image_dimension = mRenderService->getPhysicalDeviceProperties().limits.maxImageDimension2D;
		uint32_t cell_width = static_cast<uint32_t>(mWidth / mNumRows);
		uint32_t cell_height = static_cast<uint32_t>(mHeight / mNumColumns);

		// Check if the cell dimensions are supported
		if (cell_width > max_image_dimension || cell_height > max_image_dimension) {
			errorState.fail(utility::stringFormat("Image cell dimension of %dx%d not supported", cell_width, cell_height));
		}

		// Calculate number of cells required
		mNumCells = mNumRows * mNumColumns;

		mRenderTargets.resize(mNumCells);
		mBitmaps.resize(mNumCells);
		mBitmapUpdateFlags.resize(mNumCells, false);

		// Check if we need to render to BGRA for image formats on little endian machines
		// We can ignore this check when rendering 16bit color
		bool is_little_endian = FI_RGBA_RED == 2;
		bool is_8bit_4ch = (mFormat == RenderTexture2D::EFormat::RGBA8 || mFormat == RenderTexture2D::EFormat::BGRA8);
		mFormat = (is_little_endian && is_8bit_4ch) ? RenderTexture2D::EFormat::BGRA8 : RenderTexture2D::EFormat::RGBA8;

		uint32_t pixels_width_processed = 0;
		uint32_t pixels_height_processed = 0;

		for (int i=0; i<mNumCells; i++) {
			// Create render textures
			rtti::ObjectPtr<RenderTexture2D> render_texture = resourceManager->createObject<RenderTexture2D>();
			render_texture->mWidth = cell_width;
			render_texture->mHeight = cell_height;
			render_texture->mColorSpace = EColorSpace::Linear;
			render_texture->mFormat = mFormat;
			render_texture->mUsage = ETextureUsage::DynamicRead;
			render_texture->mFill = false;

			if (!render_texture->init(errorState)) {
				errorState.fail(utility::stringFormat("Failed to initialize snapshot cell texture [%d]", i));
				return false;
			}

			mRenderTargets[i] = resourceManager->createObject<RenderTarget>();
			mRenderTargets[i]->mClearColor = mClearColor;
			mRenderTargets[i]->mRequestedSamples = mRequestedSamples;
			mRenderTargets[i]->mSampleShading = false;
			mRenderTargets[i]->mColorTexture = render_texture;

			if (!mRenderTargets[i]->init(errorState)) {
				errorState.fail(utility::stringFormat("Failed to initialize snapshot cell render target [%d]", i));
				return false;
			}

			// Create bitmaps
			mBitmaps[i] = resourceManager->createObject<Bitmap>();

			// Execute whenever a bitmap is updated
			mBitmaps[i]->mBitmapUpdated.connect([this, i]() {

				// Check bitmap was actually allocated 
				if (mBitmaps[i]->empty()) {
					Logger::error("Bitmap was not initialized");
					return false;
				}

				// Keep a record of updated bitmaps
				mBitmapUpdateFlags[i] = true;

				if (mStitch) {
					// Check if all bitmaps are flagged as updated
					if (std::find(std::begin(mBitmapUpdateFlags), std::end(mBitmapUpdateFlags), false) == std::end(mBitmapUpdateFlags)) {
						onBitmapsUpdated();
						mBitmapUpdateFlags.assign(mNumCells, false);
					}
				}
				else {
					std::string path = utility::stringFormat(
						"%s/%s_%d.%s", mOutputDir.c_str(), timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S").c_str(), i + 1, extensionToString(mOutputExtension)
					);
					utility::ErrorState errorState;
					if (!mBitmaps[i]->writeToDisk(path, errorState)) {
						Logger::error("Saving image to disk failed: %s", errorState.toString().c_str());
						return false;
					}
					// Reset current bitmap updated flag
					mBitmapUpdateFlags[i] = false;
				}
				return true;
			});
		}

		// Stitch when multiple rendertargets are used
		if (mStitch && mNumCells > 1) {
			onBitmapsUpdated.connect(std::bind(&Snapshot::stitchAndSaveBitmaps, this));
		}
		return true;
	}

	bool Snapshot::takeSnapshot(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps)
	{
		if (mRenderService->beginHeadlessRecording()) {
			camera.setGridDimensions(mNumRows, mNumColumns);
			for (int i = 0; i < mNumCells; i++) {
				int x = i % mNumColumns;
				int y = i / mNumRows;

				camera.setGridLocation(y, x);
				mRenderTargets[i]->beginRendering();
				mRenderService->renderObjects(*mRenderTargets[i], camera, comps);
				mRenderTargets[i]->endRendering();
			}
			camera.setGridLocation(0, 0);
			camera.setGridDimensions(1, 1);

			mRenderService->endHeadlessRecording();

			// Save to bitmap
			for (int i = 0; i < mNumCells; i++) {
				mRenderTargets[i]->getColorTexture().asyncGetData(*mBitmaps[i]);
			}
			return true;
		}
		return false;
	}

	bool Snapshot::stitchAndSaveBitmaps()
	{
		// Store handles to unload when we are done
		std::vector<FIBITMAP*> fi_bitmap_handles;
		fi_bitmap_handles.resize(mNumCells);

		// Get format
		FREE_IMAGE_FORMAT fi_img_format = FreeImage_GetFIFFromFilename(extensionToString(mOutputExtension));
		if (fi_img_format == FIF_UNKNOWN) {
			nap::Logger::error("error: Unable to determine image format");
			return false;
		}

		// Get properties
		FREE_IMAGE_TYPE fi_img_type = utility::getFIType(mBitmaps[0]->mSurfaceDescriptor.getDataType(), mBitmaps[0]->mSurfaceDescriptor.getChannels());
		int bpp = mBitmaps[0]->mSurfaceDescriptor.getBytesPerPixel() * 8;
		int pitch = mBitmaps[0]->mSurfaceDescriptor.getPitch();

		// Allocate full bitmap to copy into
		FIBITMAP* fi_bitmap_full = FreeImage_AllocateT(fi_img_type, mWidth, mHeight, bpp);
		for (int i = 0; i < mNumCells; i++) {

			// Check if subimage format matches the others
			if (FreeImage_GetBPP(fi_bitmap_full) != bpp || mBitmaps[i]->mSurfaceDescriptor.getPitch() != pitch) {
				nap::Logger::error("error: Image format mismatch");
				return false;
			}

			// Wrap bitmap data with FIBITMAP header
			// Please note that color masks are only supported for 16-bit RGBA images and ignored for any other color depth
			FIBITMAP* fi_bitmap = FreeImage_ConvertFromRawBitsEx(
				false, (uint8_t*)mBitmaps[i]->getData(), fi_img_type, mBitmaps[i]->getWidth(), mBitmaps[i]->getHeight(), pitch, bpp,
				FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK
			);

			// Unflatten index
			int x = i % mNumColumns;
			int y = i / mNumRows;

			// Copy into full bitmap
			if (!FreeImage_Paste(fi_bitmap_full, fi_bitmap, x*mBitmaps[i]->getWidth(), y*mBitmaps[i]->getHeight(), STITCH_COMBINE)) {
				nap::Logger::error(utility::stringFormat("error: Failed to stitch subimage [%d, %d]", x, y));
				return false;
			}
			fi_bitmap_handles[i] = fi_bitmap;
		}
		// Save
		std::string path = utility::stringFormat("%s/%s.%s", mOutputDir.c_str(), timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S").c_str(), extensionToString(mOutputExtension));
		utility::ErrorState errorState;
		if (!utility::writeToDisk(fi_bitmap_full, fi_img_type, path, errorState)) {
			nap::Logger::error("error: %s", errorState.toString().c_str());
			return false;
		}

		// Unload
		for (int i = 0; i < mNumCells; i++) {
			FreeImage_Unload(fi_bitmap_handles[i]);
		}
		FreeImage_Unload(fi_bitmap_full);

		onSnapshotTaken();
		return true;
	}
}
