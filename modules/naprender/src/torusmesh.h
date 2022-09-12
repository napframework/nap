#pragma once

// External Includes
#include <nap/core.h>
#include <mesh.h>
#include <renderservice.h>

namespace nap
{
	/**
	 * Parametric TorusMesh
	 */
	class TorusMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		TorusMesh(nap::Core& core);

		/**
		 * @param errorState the error message when initialization fails
		 * @return if the component initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * Setup the mesh but do not init the mesh instance
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
		uint							mSegments = 64U;						///< Property: 'Segments' number of segments
		uint							mTubeSegments = 32U;					///< Property: 'TubeSegments' number of tubular segments
		float							mAngleOffset = 0.0f;					///< Property: 'AngleOffset' the angle offset in degrees
		EPolygonMode					mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' the polygon mode
		ECullMode						mCullMode = ECullMode::Back;			///< Property: 'CullMode' the cull mode

	protected:
		RenderService*					mRenderService = nullptr;				//< The render service
		std::unique_ptr<MeshInstance>	mMeshInstance = nullptr;				//< Runtime mesh instance
	};
}
