#pragma once

// External Includes
#include <mesh.h>

namespace nap
{
	/**
	 * Female figurine 3D mesh, located at the origin with a height of 1.
	 * The mesh has position, uv and normal vertex attributes.
	 */
	class NAPAPI LucyMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		// Constructor
		LucyMesh(Core& core);

		/**
		 * Load mesh and upload to GPU.
		 * @param errorState the error message when initialization fails
		 * @return if initialization succeeded
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the humanoid mesh instance
		 */
		virtual MeshInstance& getMeshInstance() override				{ assert(mMeshInstance != nullptr); return *mMeshInstance; }

		/**
		 * @return the humanoid mesh instance
		 */
		virtual const MeshInstance& getMeshInstance() const override	{ assert(mMeshInstance != nullptr); return *mMeshInstance; }

		EMemoryUsage	mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		EPolygonMode	mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' Mesh polygon mode (fill, wires, points)
		ECullMode		mCullMode = ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.

	private:
		RenderService* mRenderService = nullptr;
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;
	};
}
