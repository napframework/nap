/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "equirectangularcubemap.h"

// External Includes
#include <imagefromfile.h>

namespace nap
{
	/**
	 * Creates a 6 face cubemap from an equirectangular image loaded from disk.
	 * 
	 * This object must be pre-rendered at least once in a headless render pass in the first frame. The RenderAdvanced
	 * service queues a headless `RenderService::RenderCommand` for each `nap::CubeMapFromFile` in the scene after resource
	 * initialization, and will be handled when headless render commands are recorded. The code below only begins a
	 * headless recording operation only when headless commands are queued in the render service.
	 *
	 * ~~~~~{.cpp}
	 *	if (mRenderService->isHeadlessCommandQueued())
	 *	{
	 *		// Handles `nap::CubeMapFromFile` pre-render operations in the first frame
	 *		if (mRenderService->beginHeadlessRecording())
	 *			mRenderService->endHeadlessRecording();
	 *	}
	 * ~~~~~
	 */
	class NAPAPI CubeMapFromFile : public EquiRectangularCubeMap
	{
		RTTI_ENABLE(EquiRectangularCubeMap)
	public:
		// Constructor
		CubeMapFromFile(Core& core);

		/**
		 * Loads the equirectangular image from disk and schedules the conversion to a 6 face cubemap on the GPU.
		 * @param errorState contains the error when initialization fails
		 * @return true when successful, otherwise false.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::string mImagePath;				///< Property: 'ImagePath' Path to the image on disk to load

	private:
		ImageFromFile mEquiRectangularImage;
	};
}
