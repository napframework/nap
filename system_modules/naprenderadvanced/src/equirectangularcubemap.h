#pragma once

// Local includes
#include "renderadvancedservice.h"

// External includes
#include <nap/resource.h>
#include <rendertexturecube.h>
#include <texture.h>

namespace nap
{
	/**
	 * Base class for all cube maps generated from equirectangular images.
	 * Creates a 6 face cube map from an equirectangular 2D texture
	 * 
	 * If `GenerateLODs` is enabled, GPU memory for mip-maps (LODs) are
	 * allocated and will be updated using blit operations after the cube face render passes.
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
	class NAPAPI EquiRectangularCubeMap : public RenderTextureCube
	{
		RTTI_ENABLE(RenderTextureCube)
		friend class RenderAdvancedService;
	public:
		// Destructor
		virtual ~EquiRectangularCubeMap() override;

		/**
		 * @param core the core instance
		 */
		EquiRectangularCubeMap(Core& core);

		/**
		 * Uploads and renders the given equirectangular texture to cubemap.
		 * @param source the equirectangular source image, must be initialized
		 * @param errorState contains the error message when initialization failsRTTI_PROPERTY("SampleShading",		&nap::CubeMapFromFile::mSampleShading,	nap::rtti::EPropertyMetaData::Default, "Reduces texture aliasing at higher computational cost")
		 */
		virtual bool init(nap::Texture2D& equiRectangularTexture, utility::ErrorState& errorState);

		/**
		 * @return the equirectangular source image
		 */
		Texture2D& getSourceTexture()						{ assert(mEquiRectangularTexture != nullptr); return *mEquiRectangularTexture; }

		/**
		 * @return the equirectangular source image
		 */
		const Texture2D& getSourceTexture() const			{ assert(mEquiRectangularTexture != nullptr); return *mEquiRectangularTexture; }

		/**
		 * Removes the cubemap
		 */
		virtual void onDestroy();

		bool mSampleShading = false;						///< Property: 'SampleShading' Reduces texture aliasing when cubemap is generated, at higher computational cost
		bool mGenerateLods;									///< Property: 'GenerateLODs' whether to create mip-maps when the cubemap is generated

	protected:
		RenderAdvancedService* mRenderAdvancedService = nullptr;

	private:
		Texture2D* mEquiRectangularTexture = nullptr;
	};
}

