/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"
#include "renderservice.h"
#include "bitmap.h"
#include "bitmapfilebuffer.h"
#include "perspcameracomponent.h"

// External Includes
#include <nap/resource.h>
#include <nap/core.h>
#include <nap/signalslot.h>

namespace nap
{
	// Forward Declares
	class SnapshotRenderTarget;

	/**
	 * The Snapshot module renders objects at any given resolution and format and saves the result to a specified location on disk 
	 * (as long as its configuration is supported by the hardware and sufficient video memory is available).
	 */
	class NAPAPI Snapshot : public Resource
	{
		friend class SnapshotRenderTarget;
		RTTI_ENABLE(Resource)
	public:

		Snapshot(Core& core);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Updates the render target clear color.
		* @param color the new clear color to use.
		*/
		void setClearColor(const glm::vec4& color);

		/**
		* Take a high-res snapshot of the scene and save to the configured location on disk
		* Make sure begin a headless recording in the render service e.g.
		*
		* ~~~~~{.cpp}
		*				mRenderService->beginFrame();
		*				if (mRenderService->beginHeadlessRecording())
		*				{
		*					...
		*					mSnapShot->snap(camera, components_to_render);
		*					mRenderService->endHeadlessRecording();
		*				}
		* ~~~~~
		*
		* @param camera Camera to take snapshot with
		* @param comps Components to render
		*/
		bool snap(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps);

		/**
		* Returns the size of the snapshot
		* @return the size of the snapshot
		*/
		glm::u32vec2 getSize() { return { mWidth, mHeight }; };

		uint32 mWidth = 0;															///< Property: 'Width' width of the snapshot in texels
		uint32 mHeight = 0;															///< Property: 'Height' height of the snapshot in texels

		uint32 mMaxCellWidth = 1920;												///< Property: 'mMaxCellWidth' max width of a cell
		uint32 mMaxCellHeight = 1080;												///< Property: 'mMaxCellHeight' max height of a cell

		glm::vec4 mClearColor{ 0.f, 0.f, 0.f, 1.f };								///< Property: 'ClearColor' color selection used for clearing the render target
		RenderTexture2D::EFormat mTextureFormat = RenderTexture2D::EFormat::RGBA8;	///< Property: 'Format' texture format
		ERasterizationSamples mRequestedSamples = ERasterizationSamples::Four;		///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'
		bool mSampleShading = true;													///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost

		std::string mOutputDirectory = "";																///< Property: 'OutputDirectory' Location of the directory where snapshots are saved to
		BitmapFileBuffer::EImageFileFormat mImageFileFormat = BitmapFileBuffer::EImageFileFormat::PNG;	///< Property: 'ImageFormat' Image format of the snapshot image file

		// Triggered when a snapshot is being processed
		nap::Signal<> onSnapshot;

		// Triggered when a snapshot is finished being written to disk
		nap::Signal<std::string> onSnapshotSaved;

	protected:
		RenderService* mRenderService = nullptr;

	private:
		/**
		* Writes the destination bitmap to disk
		* @return true if successful
		*
		* TODO: Make this into an async task! Guard mDestBitmapFileBuffer while writing to disk.
		*/
		bool save();
		nap::Slot<> mSaveBitmapSlot = { [this]() -> void { save(); } };

		uint32 mNumRows = 1;
		uint32 mNumColumns = 1;

		uint32 mNumCells = 0;
		glm::u32vec2 mCellSize = { 0, 0 };

		std::vector<std::unique_ptr<RenderTexture2D>> mColorTextures;
		std::unique_ptr<SnapshotRenderTarget> mRenderTarget;

		std::vector<bool> mCellUpdateFlags;											//< List of update flags for each cell
		nap::Signal<> onCellsUpdated;												//< Triggered when all cells are updated

		std::unique_ptr<BitmapFileBuffer> mDestBitmapFileBuffer;					//< The destination bitmap file buffer
	};
}
