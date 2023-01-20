#pragma once

// External Includes
#include <mesh.h>

namespace nap
{
    // Forward Declares
	class RenderService;

	/**
	 * Resource to generate a mesh instance without buffers.
	 * For simple rendering pipelines that rely on shader programs only.
	 */
	class NAPAPI NoMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		NoMesh(Core& core);

		/**
		 * Initialize this dummy mesh
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual MeshInstance& getMeshInstance()	override { return *mMeshInstance; }

		/**
		 * @return MeshInstance as created during init().
		 */
		virtual const MeshInstance& getMeshInstance() const	override { return *mMeshInstance; }

	private:
		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;			///< The mesh instance to construct
		nap::RenderService* mRenderService = nullptr;					///< Handle to the render service
	};
}
