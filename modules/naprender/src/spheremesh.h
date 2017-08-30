#pragma once

#include "mesh.h"
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * Predefined sphere mesh
	 */
	class NAPAPI SphereMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)

	public:
		/**
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }
		virtual const MeshInstance& getMeshInstance() const { return *mMeshInstance; }

	public:
		float mRadius	= 1.0f;		// The radius of the mesh
		float mRings	= 50.0f;	// The number of rings in the mesh
		float mSectors	= 50.0f;	// The number of sectors in the mesh

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
