/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"

// External Includes
#include <nap/resource.h>
#include <nap/core.h>

#include <renderservice.h>
#include <bitmap.h>

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

		bool beginSnapshot();
		void endSnapshot();

		RenderTarget& getSnapshotRenderTarget();

		int	mWidth = 0;		///< Property: 'Width' width of the texture in texels
		int	mHeight = 0;	///< Property: 'Height' of the texture in texels

	protected:
		RenderService* mRenderService = nullptr;

	private:
		rtti::ObjectPtr<RenderTarget> mRenderTarget;
		Bitmap mBitmap;

		using SaveScreenshotCallback = std::function<void()>;
		SaveScreenshotCallback mSaveSnapshotCallback;

		bool mBusy = false;
	};
}
