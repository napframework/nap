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
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

	public:
		float mRadius	= 1.0f;		///< Property: 'Radius' of the sphere
		float mRings	= 50.0f;	///< Property: 'Rings' number of rings
		float mSectors	= 50.0f;	///< Property: 'Sectors' number of sectors

	private:
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
