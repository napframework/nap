#pragma once

// Local Includes
#include "mesh.h"

namespace nap
{
	/**
	 * Predefined plane mesh with size 1x1, centered at origin.
	 */
	class NAPAPI PlaneMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)

	public:
		/**
		 * Creates the mesh.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
