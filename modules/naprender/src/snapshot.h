/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"

// External Includes
#include <nap/resource.h>
#include <nap/core.h>
#include <nap/signalslot.h>

#include <renderservice.h>
#include <bitmap.h>
#include <rect.h>
#include <perspcameracomponent.h>
//#include <utility/threading.h>

namespace nap
{
	/**
	 * Snapshot
	 */
	class NAPAPI Snapshot : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		Snapshot(Core& core);
		virtual ~Snapshot();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Take a high-res snapshot of the scene and save to the configured location on disk
		* Todo: Support ortho cams later
		* @param camera Camera to take snapshot with
		* @param comps Components to render
		*/
		bool takeSnapshot(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps);


		uint32_t mWidth = 0;													///< Property: 'Width' width of the snapshot in texels
		uint32_t mHeight = 0;													///< Property: 'Height' height of the snapshot in texels
		uint32_t mNumRows = 0;													///< Property: 'DesiredCellWidth' desired width of a cell
		uint32_t mNumColumns = 0;												///< Property: 'DesiredCellHeight' desired height of a cell

		glm::vec4 mClearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);					///< Property: 'ClearColor' color selection used for clearing the render target
		RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8;		///< Property: 'Format' texture format
		ERasterizationSamples mRequestedSamples = ERasterizationSamples::One;	///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		bool mSampleShading = false;											///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.

		std::string mOutputDir = "";											///< Property: 'OutputPath' Location of the directory where snapshots are saved to
		std::string mOutputExtension = "";										///< Property: 'OutputExtension' Extension of the snapshot image file e.g. "png", "jpg" or "tiff"

	protected:
		RenderService* mRenderService = nullptr;

	private:
		std::vector<rtti::ObjectPtr<RenderTarget>> mRenderTargets;
		std::vector<rtti::ObjectPtr<Bitmap>> mBitmaps;

		uint32_t mNumCells = 0;

		//std::unique_ptr<BitmapWriteThread> mBitmapWriteThread;
	};

 	//class BitmapWriteThread : public WorkerThread
 	//{
 	//public:
 	//	BitmapWriteThread(bool blocking = false, std::uint32_t maxQueueItems = 1) : WorkerThread(blocking, maxQueueItems) {}
 	//};
}
