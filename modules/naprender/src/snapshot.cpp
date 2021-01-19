/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "snapshot.h"

// nap::Snapshot run time class definition 
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
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	Snapshot::Snapshot(Core& core) :
		mRenderService(core.getService<RenderService>()) {}

	Snapshot::~Snapshot() {}

	bool Snapshot::init(utility::ErrorState& errorState)
	{
		assert(mNumRows > 0 && mNumColumns > 0);
		assert(mWidth > mNumRows && mHeight > mNumColumns);

		nap::ResourceManager* resourceManager = mRenderService->getCore().getResourceManager();

		uint32_t max_image_dimension = mRenderService->getPhysicalDeviceProperties().limits.maxImageDimension2D;
		uint32_t cell_width = static_cast<uint32_t>(mWidth / mNumRows);
		uint32_t cell_height = static_cast<uint32_t>(mHeight / mNumColumns);

		// Check if the cell dimensions are supported
		if (max_image_dimension > cell_width || max_image_dimension > cell_height) {
			errorState.fail(utility::stringFormat("Image cell dimension of %dx%d not supported", cell_width, cell_height));
		}

		// Calculate number of cells required
		mNumCells = mNumRows * mNumColumns;

		mRenderTargets.resize(mNumCells);
		mBitmaps.resize(mNumCells);

		uint32_t pixels_width_processed = 0;
		uint32_t pixels_height_processed = 0;

		for (int y = 0; y < mNumColumns; y++) {
			pixels_width_processed = 0;

			for (int x = 0; x < mNumRows; x++) {
				int cell_index = y * mNumRows + x;

				// Create render texture
				rtti::ObjectPtr<RenderTexture2D> render_texture = resourceManager->createObject<RenderTexture2D>();
				render_texture->mWidth = cell_width;
				render_texture->mHeight = cell_height;
				render_texture->mColorSpace = EColorSpace::Linear;
				render_texture->mFormat = mFormat; // BGRA8 -- not implemented for RTs, give control to user
				render_texture->mUsage = ETextureUsage::DynamicRead;
				render_texture->mFill = false;

				if (!render_texture->init(errorState)) {
					errorState.fail(utility::stringFormat("Failed to initialize snapshot cell texture [%d, %d]", x, y));
					return false;
				}

				mRenderTargets[cell_index] = resourceManager->createObject<RenderTarget>();
				mRenderTargets[cell_index]->mClearColor = mClearColor;
				mRenderTargets[cell_index]->mRequestedSamples = mRequestedSamples;
				mRenderTargets[cell_index]->mSampleShading = false;
				mRenderTargets[cell_index]->mColorTexture = render_texture;

				if (!mRenderTargets[cell_index]->init(errorState)) {
					errorState.fail(utility::stringFormat("Failed to initialize snapshot cell render target [%d, %d]", x, y));
					return false;
				}

				// Create bitmaps
				mBitmaps[cell_index] = resourceManager->createObject<Bitmap>();

				// Connect Bitmap write task to BitmapBownloaded signal
				mBitmaps[cell_index]->mBitmapUpdated.connect([this, i = cell_index]() {
					utility::ErrorState errorState;
					if (mBitmaps[i]->empty()) {
						errorState.fail("Saving image to disk failed: bitmap not initialized");
						return;
					}
					std::string path = utility::stringFormat("%s/%s_%d.%s", mOutputDir.c_str(), timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S").c_str(), i + 1, mOutputExtension.c_str());
					if (!mBitmaps[i]->writeToDisk(path, errorState)) {
						std::cout << "Saving image to disk failed: " << errorState.toString();
					}
					//else {
					//	mBitmapWriteThread->enqueue([this](void) {
					//		utility::ErrorState error;
					//		if (!mBitmap.writeToDisk(utility::stringFormat("%s.png", timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S").c_str()), error)) {
					//			std::cout << "Saving image to disk failed: " << error.toString();
					//		}
					//	});
					//}
				});
				pixels_width_processed += cell_width;
			}
			pixels_height_processed += cell_height;
		}

		//mBitmapWriteThread = std::make_unique<BitmapWriteThread>();
		//mBitmapWriteThread->start();

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
}
