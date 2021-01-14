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

		bool takeSnapshot(CameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps);

		RenderTexture2D& getColorTexture();

		int	mWidth = 0;				///< Property: 'Width' width of the texture in texels
		int	mHeight = 0;			///< Property: 'Height' of the texture in texels
		glm::vec4 mClearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);

	protected:
		RenderService* mRenderService = nullptr;

	private:
		rtti::ObjectPtr<RenderTarget> mRenderTarget;
		Bitmap mBitmap;

		bool mBusy = false;

		/**
		* Called by the slot when the bitmap is updated from a RenderTarget
		*/
		void onDownloadReady(const BitmapDownloadedEvent& event);

		// Slot that is called when the rendertarget's texture is read back from the gpu
		NSLOT(mDownloadReady, const BitmapDownloadedEvent&, onDownloadReady);
	};
}
