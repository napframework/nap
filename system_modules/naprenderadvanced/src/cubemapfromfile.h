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
	 */
	class NAPAPI CubeMapFromFile : public RenderTextureCube
	{
		RTTI_ENABLE(RenderTextureCube)
	public:
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
		Texture2D& getSourceTexture() { return *mSourceImage; }

	public:
		std::string					mImagePath;								///< Property: 'ImagePath' Path to the image on disk to load
		bool						mSampleShading = false;					///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost

		using RenderTextureCube::mGenerateLODs;								///< Property: 'GenerateLODs' whether to use and update mip-maps each time the cube texture is updated

	private:
		std::unique_ptr<Image> mSourceImage;
	};
}
