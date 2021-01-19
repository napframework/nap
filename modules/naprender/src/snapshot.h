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

		// For testing purposes only. Will remove later
		RenderTexture2D& getColorTexture();

		int	mWidth = 0;					///< Property: 'Width' width of the snapshot in texels
		int	mHeight = 0;				///< Property: 'Height' height of the snapshot in texels
		int mDesiredCellWidth = 0;		///< Property: 'DesiredCellWidth' desired width of a cell
		int mDesiredCellHeight = 0;		///< Property: 'DesiredCellHeight' desired height of a cell

		glm::vec4 mClearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);

	protected:
		RenderService* mRenderService = nullptr;

	private:
		std::vector<rtti::ObjectPtr<RenderTarget>> mRenderTargets;
		std::vector<rtti::ObjectPtr<Bitmap>> mBitmaps;
		std::vector<math::Rect> mViewportRects;

		int mNumRows = 0;
		int mNumColumns = 0;
		int mNumCells = 0;

		//std::unique_ptr<BitmapWriteThread> mBitmapWriteThread;
	};

 	//class BitmapWriteThread : public WorkerThread
 	//{
 	//public:
 	//	BitmapWriteThread(bool blocking = false, std::uint32_t maxQueueItems = 1) : WorkerThread(blocking, maxQueueItems) {}
 	//};
}
