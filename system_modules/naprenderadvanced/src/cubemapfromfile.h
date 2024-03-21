/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rendertexturecube.h>
#include <image.h>

// Local includes
#include "cuberendertarget.h"

namespace nap
{
	// Forward Declares
	class Core;

	/**
	 * Loads equirectangular image from disk, uploads it to the GPU, and schedules a rendering operation to generate a cube
	 * map from it in the `nap::RenderAdvancedService`. If `GenerateLODs` is enabled, GPU memory for mip-maps (LODs) are
	 * allocated and will be updated using blit operations after the cube face render passes.
	 *
	 * This object must be pre-rendered at least once in a headless render pass in the first frame. The RenderAdvanced service
	 * queues a `HeadlessRenderCommand` for each `nap::CubeMapFromFile` in the scene after resource initialization, and will
	 * be handled when headless render commands are recorded. The code below only begins a headless recording operation only
	 * when headless commands are queued in the render service.
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
	class NAPAPI CubeMapFromFile : public RenderTextureCube
	{
		RTTI_ENABLE(RenderTextureCube)
	public:
		friend class RenderAdvancedService;

		// Destructor
		virtual ~CubeMapFromFile() {}

		/**
		 * @param core the core instance
		 */
		CubeMapFromFile(Core& core);

		/**
		 * Loads the image from disk and schedules the upload to the GPU on success.
		 * @param errorState contains the error when initialization fails
		 * @return true when successful, otherwise false.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the source texture
		 */
		Texture2D& getSourceTexture() const				{ return *mSourceImage; }

		/**
		 * Returns whether the cube map must be (re-)rendered by the render advanced service e.g. after a hot-reload.
		 * Only this resource and the render advanced service can set the flag.
		 * @return dirty flag
		 */
		bool isDirty() const							{ return mDirty; }

	public:
		std::string					mImagePath;								///< Property: 'ImagePath' Path to the image on disk to load
		bool						mSampleShading = false;					///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost

		using RenderTextureCube::mGenerateLODs;								///< Property: 'GenerateLODs' whether to use and update mip-maps each time the cube texture is updated

	private:
		std::unique_ptr<Image> mSourceImage;
		bool mDirty = true;
	};
}
