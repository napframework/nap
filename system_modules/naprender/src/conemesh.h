#pragma once

// External Includes
#include <nap/core.h>
#include <mesh.h>
#include <renderservice.h>

namespace nap
{
	/**
	 * Predefined cone mesh with additional normal, uv and color vertex attributes.
	 */
	class ConeMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		ConeMesh(nap::Core& core);

		/**
		 * Creates and initializes the mesh on the GPU. 
		 * @param errorState contains the error if initialization fails.
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates the cone without GPU representation. 
		 * @param errorState contains the error if initialization fails.
		 */
		bool setup(utility::ErrorState& errorState);

		/**
		 *	@return the mesh instance
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }

		/**
		 *	@return the mesh instance
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

		float							mHeight = 1.0f;							///< Property: 'Height' cone height
		float							mBottomRadius = 1.0f;					///< Property: 'BottomRadius' cone bottom radius
		float							mTopRadius = 0.0f;						///< Property: 'TopRadius' cone top radius
		uint							mSegments = 64U;						///< Property: 'Segments' number of segments
		float							mAngleOffset = 0.0f;					///< Property: 'AngleOffset' the angle offset in degrees
		EMemoryUsage					mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		EPolygonMode					mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' the polygon mode
		ECullMode						mCullMode = ECullMode::Back;			///< Property: 'CullMode' the cull mode
		RGBAColorFloat					mColor = { 1.0f, 1.0f, 1.0f, 1.0f };	///< Property: 'Color' Vertex color
		bool							mFlipNormals = false;					///< Property: 'FlipNormals' Whether to flip the cone normals such that they point inwards.

	protected:
		RenderService*					mRenderService = nullptr;				//< The render service
		std::unique_ptr<MeshInstance>	mMeshInstance = nullptr;				//< Runtime mesh instance
	};
}
