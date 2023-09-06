#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>
#include <rendertexturecube.h>

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

	public:
		ResourcePtr<Texture2D>		mSourceTexture;							///< Property: 'SourceTexture' Texture to convert to cube map
		bool						mSampleShading = false;					///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost
	};
}
