/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "mesh.h"
#include <utility/dllexport.h>

namespace nap
{
	class Core;

	/**
	 * Predefined 3D spherical mesh.
	 */
	class NAPAPI SphereMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)

	public:
		SphereMesh(Core& core);

		/**
 		 * Creates the mesh.
		 * @param errorState contains the error if the mesh can't be created.
		 * @return if the mesh was created successfully. 
 		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh instance that can be rendered
		 */
		virtual MeshInstance& getMeshInstance() override { return *mMeshInstance; }
		
		/**
		 * @return the mesh instance that can be rendered
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; }

	public:
		float			mRadius	= 1.0f;							///< Property: 'Radius' of the sphere
		float			mRings = 50.0f;							///< Property: 'Rings' number of rings
		float			mSectors = 50.0f;						///< Property: 'Sectors' number of sectors
		EMeshDataUsage	mUsage = EMeshDataUsage::Static;		///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode = ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
