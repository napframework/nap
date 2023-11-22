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
	 * CubeMapFromFile
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
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
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
