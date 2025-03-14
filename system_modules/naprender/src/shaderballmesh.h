#pragma once

// External Includes
#include <mesh.h>

namespace nap
{
	/**
	 * 3D shaderball mesh, located at the origin with a height of 1.
	 * This mesh is typically used to preview shaders.
	 * 
	 * The mesh has position, uv, normal and color vertex attributes.
	 * The color attribute can be broken up into 3 channels, where
	 * Red(0) is the center, Green(1) is the enclosing shape around it and Blue(2) is the base.
	 * 
	 * The UV coordinates of this mesh extend beyond the 0–1 range and require tiling.
	 * To ensure proper texture repetition, set both the 'AddressModeHorizontal' and 'AddressModeVertical' in your sampler to 'Repeat'.
	 */
	class NAPAPI ShaderBallMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		// Constructor
		ShaderBallMesh(Core& core);

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
