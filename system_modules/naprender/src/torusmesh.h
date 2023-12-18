#pragma once

// External Includes
#include <nap/core.h>
#include <mesh.h>
#include <renderservice.h>

namespace nap
{
	/**
	 * Predefined torus mesh with additional normal, uv and color vertex attributes.
	 */
	class TorusMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		TorusMesh(nap::Core& core);

		/**
		 * Creates and initializes the mesh on the GPU. 
		 * @param errorState contains the error if initialization fails.
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Creates the torus without GPU representation. 
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

		float							mRadius = 0.75f;						///< Property: 'Radius' torus radius
		float							mTubeRadius = 0.25f;					///< Property: 'Height' tube segment radius
		uint							mSegments = 64;							///< Property: 'Segments' number of segments
		uint							mTubeSegments = 64;						///< Property: 'TubeSegments' number of tubular segments
		float							mAngleOffset = 0.0f;					///< Property: 'AngleOffset' the angle offset in degrees
		EMemoryUsage					mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		EPolygonMode					mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' the polygon mode
		ECullMode						mCullMode = ECullMode::Back;			///< Property: 'CullMode' the cull mode
		RGBAColorFloat					mColor = { 1.0f, 1.0f, 1.0f, 1.0f };	///< Property: 'Color' Vertex color

	protected:
		RenderService*					mRenderService = nullptr;				//< The render service
		std::unique_ptr<MeshInstance>	mMeshInstance = nullptr;				//< Runtime mesh instance
	};
}
