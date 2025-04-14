// Local includes
#include "shaderballmesh.h"
#include "geometryfromfile.h"

// External includes
#include <nap/core.h>
#include <renderservice.h>

// nap::humanoidmesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ShaderBallMesh, "3D shaderball mesh, located at the origin with a height of 1.")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::ShaderBallMesh::mUsage,			nap::rtti::EPropertyMetaData::Default,	"If the mesh is static or frequently updated")
	RTTI_PROPERTY("CullMode",		&nap::ShaderBallMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default,	"Controls which triangles are culled, back facing, front facing etc.")
	RTTI_PROPERTY("PolygonMode",	&nap::ShaderBallMesh::mPolygonMode,		nap::rtti::EPropertyMetaData::Default,	"Mesh polygon draw mode (fill, wires, points)")
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

namespace nap
{
	ShaderBallMesh::ShaderBallMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool ShaderBallMesh::init(utility::ErrorState& errorState)
	{
		// Find asset
		assert(mRenderService != nullptr);
		static constexpr const char* humanoid_asset = "meshes/shaderball.fbx";
		auto asset_path = mRenderService->getModule().findAsset(humanoid_asset);
		if (!errorState.check(!asset_path.empty(), "Unable to locate asset: %s", humanoid_asset))
			return false;

		// Create import settings
		GeometryFromFile::ImportSettings settings;
		settings.mGenerateNormals  = false;
		settings.mGenerateTangents = false;

		// Load humanoid
		auto mesh_instance = std::make_unique<nap::MeshInstance>(*mRenderService);
		if (!GeometryFromFile::load(asset_path, mesh_instance, settings, errorState))
			return false;

		// Setup GPU properties
		mesh_instance->setCullMode(mCullMode);
		mesh_instance->setPolygonMode(mPolygonMode);
		mesh_instance->setUsage(mUsage);

		// Initialize for rendering
		if (!mesh_instance->init(errorState))
			return false;

		// Store
		mMeshInstance = std::move(mesh_instance);
		return true;
	}
}

