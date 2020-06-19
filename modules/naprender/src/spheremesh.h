#pragma once

#include "mesh.h"
#include <utility/dllexport.h>

namespace nap
{
	class Core;

	/**
	 * Predefined sphere mesh
	 */
	class NAPAPI SphereMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)

	public:
		SphereMesh(Core& core);

		/**
 		 * Load the mesh
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }
		
		/**
		 * @return the mesh instance that can be rendered to screen
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

	public:
		float mRadius	= 1.0f;		///< Property: 'Radius' of the sphere
		float mRings	= 50.0f;	///< Property: 'Rings' number of rings
		float mSectors	= 50.0f;	///< Property: 'Sectors' number of sectors

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
