/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"

// External Includes
#include <utility/dllexport.h>
#include <color.h>

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
		uint			mRings = 50;							///< Property: 'Rings' number of rings
		uint			mSectors = 50;							///< Property: 'Sectors' number of sectors
		EMemoryUsage	mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode = ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.
		EPolygonMode	mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)
		RGBAColorFloat	mColor = { 1.0f, 1.0f, 1.0f, 1.0f };	///< Property: 'Color' the vertex color of the sphere
		glm::vec3		mPosition = { 0.0, 0.0, 0.0 };			///< Property: 'Position' center of the sphere

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;
	};
}
