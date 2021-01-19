/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "snapshot.h"

// nap::Snapshot run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Snapshot)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width", &nap::Snapshot::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::Snapshot::mHeight, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("DesiredCellWidth", &nap::Snapshot::mDesiredCellWidth, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DesiredCellHeight", &nap::Snapshot::mDesiredCellHeight, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	Snapshot::Snapshot(Core& core) :
		mRenderService(core.getService<RenderService>()) {}

	Snapshot::~Snapshot() {}

	bool Snapshot::init(utility::ErrorState& errorState)
	{
		nap::ResourceManager* resourceManager = mRenderService->getCore().getResourceManager();

		// Max image dimension supported
		// This does not seem to be the call we are looking for. 
		uint32_t max_image_dimension = mRenderService->getPhysicalDeviceProperties().limits.maxImageDimension2D;

		// Todo: Remove later
		const uint32_t max_image_dimension_hardcoded = 4096;
		std::cout << "max_image_dimension: " << max_image_dimension_hardcoded << '/' << max_image_dimension << std::endl;

		max_image_dimension = max_image_dimension_hardcoded;

		//if (mDesiredCellWidth > max_image_dimension) {

		//}

		// Calculate number of cells required
		mNumRows = ceil(mWidth/static_cast<float>(max_image_dimension));
		mNumColumns = ceil(mHeight/static_cast<float>(max_image_dimension));
		mNumCells = mNumRows * mNumColumns;

		mRenderTargets.resize(mNumCells);
		mViewportRects.resize(mNumCells);
		mBitmaps.resize(mNumCells);

		uint32_t pixels_width_processed = 0;
		uint32_t pixels_height_processed = 0;

		for (int y = 0; y < mNumColumns; y++) {
			pixels_width_processed = 0;
			uint32_t chunk_size_height = std::min(max_image_dimension, mHeight - pixels_height_processed);

			for (int x = 0; x < mNumRows; x++) {
				uint32_t chunk_size_width = std::min(max_image_dimension, mWidth - pixels_width_processed);
				int cell_index = y * mNumRows + x;

				// Create render texture
				rtti::ObjectPtr<RenderTexture2D> render_texture = resourceManager->createObject<RenderTexture2D>();
				render_texture->mWidth = chunk_size_width;
				render_texture->mHeight = chunk_size_height;
				render_texture->mColorSpace = EColorSpace::Linear;
				render_texture->mFormat = RenderTexture2D::EFormat::RGBA8;
				render_texture->mUsage = ETextureUsage::DynamicRead;
				render_texture->mFill = false;

				if (!render_texture->init(errorState)) {
					std::cout << "failed to initialize screenshot texture" << errorState.toString();
					return false;
				}

				mRenderTargets[cell_index] = resourceManager->createObject<RenderTarget>();
				mRenderTargets[cell_index]->mClearColor = mClearColor;
				mRenderTargets[cell_index]->mRequestedSamples = ERasterizationSamples::One;
				mRenderTargets[cell_index]->mSampleShading = false;
				mRenderTargets[cell_index]->mColorTexture = render_texture;

				if (!mRenderTargets[cell_index]->init(errorState)) {
					std::cout << "failed to initialize screenshot render target" << errorState.toString();
					return false;
				}

				// Store normalized viewport coordinates
				mViewportRects[cell_index] = math::Rect(
					pixels_width_processed/static_cast<float>(mWidth),
					pixels_height_processed/static_cast<float>(mHeight),
					chunk_size_width/static_cast<float>(mWidth),
					chunk_size_height/static_cast<float>(mHeight)
				);
				//printf("(%.04f, %.04f)_(%.04f, %.04f)\n", 
				//	mViewportRects[cell_index].getMin().x,
				//	mViewportRects[cell_index].getMin().y, 
				//	mViewportRects[cell_index].getMax().x, 
				//	mViewportRects[cell_index].getMax().y
				//);

				// Create bitmaps
				mBitmaps[cell_index] = resourceManager->createObject<Bitmap>();

				// Connect Bitmap write task to BitmapBownloaded signal
				mBitmaps[cell_index]->mBitmapDownloaded.connect([this, i = cell_index]() {
					utility::ErrorState error;
					if (mBitmaps[i]->empty()) {
						std::cout << "Saving image to disk failed: Bitmap not initialized";
					}
					else if (!mBitmaps[i]->writeToDisk(utility::stringFormat("%s_%d.png", timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S").c_str(), i+1), error)) {
						std::cout << "Saving image to disk failed: " << error.toString();
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

				pixels_width_processed += max_image_dimension;
			}
			pixels_height_processed += max_image_dimension;
		}

		//mBitmapWriteThread = std::make_unique<BitmapWriteThread>();
		//mBitmapWriteThread->start();

		return true;
	}

	bool Snapshot::takeSnapshot(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps)
	{
		if (mRenderService->beginHeadlessRecording()) {
			for (int i = 0; i < mNumCells; i++) {

				// Build new camera matrix based on viewport
				camera.setProjectionMatrixCell(mViewportRects[i]);

				mRenderTargets[i]->beginRendering();
				mRenderService->renderObjects(*mRenderTargets[i], camera, comps);
				mRenderTargets[i]->endRendering();
			}
			mRenderService->endHeadlessRecording();

			// Save to bitmap
			for (int i = 0; i < mNumCells; i++) {
				mRenderTargets[i]->getColorTexture().asyncGetData(*mBitmaps[i]);
			}
			return true;
		}
		return false;
	}

	RenderTexture2D& Snapshot::getColorTexture()
	{
		return mRenderTargets[0]->getColorTexture();
	}
}
