/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"
#include "renderservice.h"
#include "bitmap.h"
#include "bitmaputils.h"
#include "perspcameracomponent.h"

// External Includes
#include <nap/resource.h>
#include <nap/core.h>
#include <nap/signalslot.h>
#include <future>

namespace nap
{
	/**
	 * The Snapshot module renders objects at any given resolution and format and saves the result to a specified location on disk 
	 * (as long as its configuration is supported by the hardware and sufficient video memory is available).
	 */
	class NAPAPI Snapshot : public Resource
	{
		RTTI_ENABLE(Resource)
		friend class SnapshotRenderTarget;
	public:

		/**
		* All supported output extensions for snapshots
		*/
		enum class EOutputExtension
		{
			PNG,					///< Portable Network Graphics (*.PNG)
			JPG,					///< Independent JPEG Group (*.JPG, *.JIF, *.JPEG, *.JPE)
			TIFF,					///< Tagged Image File Format (*.TIF, *.TIFF)
			BMP						///< Windows or OS/2 Bitmap File (*.BMP)
		};

		Snapshot(Core& core);
		~Snapshot() override;

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
		* @param camera Camera to take snapshot with
		* @param comps Components to render
		*/
		bool takeSnapshot(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps);

		/**
		* Returns the size of the snapshot
		* @return the size of the snapshot
		*/
		glm::u32vec2 getSize() { return { mWidth, mHeight }; };

		/**
		* Returns a string representation of the configured output extension
		* @return the output extension
		*/
		std::string getExtension();


		uint32_t mWidth = 0;													///< Property: 'Width' width of the snapshot in texels
		uint32_t mHeight = 0;													///< Property: 'Height' height of the snapshot in texels

		uint32_t mMaxCellWidth = 1920;											///< Property: 'mMaxCellWidth' max width of a cell
		uint32_t mMaxCellHeight = 1080;											///< Property: 'mMaxCellHeight' max height of a cell

		glm::vec4 mClearColor{ 0.f, 0.f, 0.f, 1.f };							///< Property: 'ClearColor' color selection used for clearing the render target
		RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8;		///< Property: 'Format' texture format
		ERasterizationSamples mRequestedSamples = ERasterizationSamples::Four;	///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		bool mSampleShading = true;												///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.

		std::string mOutputDir = "";											///< Property: 'OutputPath' Location of the directory where snapshots are saved to.
		EOutputExtension mOutputExtension = EOutputExtension::PNG;				///< Property: 'OutputExtension' Extension of the snapshot image file.

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
		* TODO: Make this into an async task! Guard mDstBitmap while writing to disk.
		*/
		bool writeToDisk();
		nap::Slot<> writeBitmapSlot = { [this]() -> void { writeToDisk(); } };

		uint32_t mNumRows = 1;
		uint32_t mNumColumns = 1;

		uint32_t mNumCells = 0;
		glm::u32vec2 mCellSize = { 0, 0 };

		std::vector<std::unique_ptr<RenderTexture2D>> mColorTextures;
		std::unique_ptr<SnapshotRenderTarget> mRenderTarget;

		std::unique_ptr<utility::FIBitmapInfo> mFIBitmapInfo;
		std::vector<bool> mBitmapUpdateFlags;

		// Triggered when all cells are updated
		nap::Signal<> onCellsUpdated;

		// Destination bitmap
		FIBITMAP* mDstBitmap = nullptr;
		bool mDstBitmapAllocated = false;
	};
}
