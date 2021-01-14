/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "snapshot.h"

// nap::Snapshot run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Snapshot)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Width", &nap::Snapshot::mWidth, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height", &nap::Snapshot::mHeight, nap::rtti::EPropertyMetaData::Required)
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

		rtti::ObjectPtr<RenderTexture2D> renderTex = resourceManager->createObject<RenderTexture2D>();
		renderTex->mWidth = mWidth;
		renderTex->mHeight = mHeight;
		renderTex->mColorSpace = EColorSpace::Linear;
		renderTex->mFormat = RenderTexture2D::EFormat::RGBA8;
		renderTex->mUsage = ETextureUsage::DynamicRead;
		renderTex->mFill = false;

		if (!renderTex->init(errorState)) {
			std::cout << "failed to initialize screenshot texture" << errorState.toString();
			return false;
		}

		mRenderTarget = resourceManager->createObject<RenderTarget>();
		mRenderTarget->mClearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);
		mRenderTarget->mRequestedSamples = ERasterizationSamples::Four;
		mRenderTarget->mSampleShading = true;
		mRenderTarget->mColorTexture = renderTex;

		if (!mRenderTarget->init(errorState)) {
			std::cout << "failed to initialize screenshot render target" << errorState.toString();
			return false;
		}

		// Write screenshot to disk. This must be queued to happen when the texture download has completed.
		mSaveSnapshotCallback = [this](void) {
			utility::ErrorState error;
			if (mBitmap.empty()) {
				std::cout << "Saving image to disk failed: Bitmap not initialized";
			}
			else if (!mBitmap.writeToDisk(utility::stringFormat("%s.png", timeFormat(getCurrentTime(), "%Y%m%d_%H%M%S").c_str()), error)) {
				std::cout << "Saving image to disk failed: " << error.toString();
			}
			mBusy = false;
		};
		return true;
	}

	bool Snapshot::beginSnapshot()
	{
		if (mRenderService->beginHeadlessRecording()) {
			mRenderTarget->beginRendering();
			return true;
		}
		return false;
	}

	void Snapshot::endSnapshot()
	{
		mRenderTarget->endRendering();
		mRenderService->endHeadlessRecording();

		// Save to bitmap
		mRenderTarget->getColorTexture().asyncGetData(mBitmap, mSaveSnapshotCallback);
		mBusy = true;
	}

	RenderTarget& Snapshot::getSnapshotRenderTarget()
	{
		return *mRenderTarget;
	}
}