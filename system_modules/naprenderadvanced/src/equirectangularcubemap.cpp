// Local includes
#include "equirectangularcubemap.h"
#include "renderadvancedservice.h"

// External includes
#include <nap/core.h>

// nap::equirectangularcubemap run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::EquiRectangularCubeMap)
	RTTI_PROPERTY("SampleShading",	&nap::EquiRectangularCubeMap::mSampleShading,	nap::rtti::EPropertyMetaData::Default, "Reduces texture aliasing when cubemap is generated, at higher computational cost")
	RTTI_PROPERTY("GenerateLODs",	&nap::EquiRectangularCubeMap::mGenerateLods,	nap::rtti::EPropertyMetaData::Default, "Create and use mip-maps when cube-map is generated from equirectangular image")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	EquiRectangularCubeMap::~EquiRectangularCubeMap()
	{
		// Explicitly destroy cubemap when created manually
		if (mEquiRectangularTexture != nullptr)
			onDestroy();
	}


	EquiRectangularCubeMap::EquiRectangularCubeMap(Core& core) : RenderTextureCube(core),
		mRenderAdvancedService(core.getService<nap::RenderAdvancedService>())
	{ }


	bool EquiRectangularCubeMap::init(nap::Texture2D& source, utility::ErrorState& errorState)
	{
		// Initialize base
		if (!RenderTextureCube::init(mGenerateLods, errorState))
			return false;

		// Register and store
		mRenderAdvancedService->registerEquiRectangularCubeMap(*this);
		mEquiRectangularTexture = &source;
		return true;
	}


	void EquiRectangularCubeMap::onDestroy()
	{
		mRenderAdvancedService->removeEquiRectangularCubeMap(*this);
		mEquiRectangularTexture = nullptr;
	}
}
