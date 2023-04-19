/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

 // Local Includes
#include "mesh.h"

// External Includes
#include <rtti/object.h>
#include <imagefromfile.h>
#include <nap/resourceptr.h>

namespace nap
{
	class Core;

	/**
	 * Height Mesh Resource
	 * This resource is derived from a plane where the plance vertices are displaced along their normal based on a height map
	 * The height map is a link to an image where the red component is sampled for it's height information
	 * 8, 16bit and 32 bit height maps all work. Colors are converted to float values on initialization
	 * This resource also creates a set of extra vertex attributes that can be used in a shader: "OriginalPosition" and "OriginalNormal"
	 * The original position vertex attribute stores the original plane (pre-displacement) vertex positions, useful for blending positions later on
	 * The original normal vertex attribute stores the original plane (pre-displacement) vertex normals, useful for blending the normal later on
	 * The end result is a plane where the vertices are displaced along it's normal based on the height data in the height map.
	 * When you displace a vertex the normals need to be recomupted based on it's surrounding triangles. This happens after displacement in the init() call
	 * This ensures the mesh has correct normals for light / effect calculations inside a shader
	 */
	class HeightSphereMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		HeightSphereMesh(Core& core);

		/**
		 * Initialize this object after de-serialization
		 * @param errorState contains the error message when initialization fails
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the mesh instance
		 */
		virtual MeshInstance& getMeshInstance() override		{ return *mMeshInstance; };

		/**
		 * @return the mesh instance
		 */
		virtual const MeshInstance& getMeshInstance() const override { return *mMeshInstance; };

		// Height map Properties
		ResourcePtr<ImageFromFile> mHeightmap;					///< Property: "Heightmap" image resource
		float mHeight = 1.0f;									///< Property: "Height" max elevation level applied to the mesh with a pixel value of 1

		/**
		 * @return the original plane vertex positions
		 */
		Vec3VertexAttribute& getOriginalPosition() const		{ return *mOriginalPosAttr; }
		
		/**
		 * @return the original plane normals
		 */
		Vec3VertexAttribute& getOriginalNormals() const			{ return *mOriginalNorAttr; }

		/**
		 *	@return the displaced position
		 */
		Vec3VertexAttribute& getDisplacedPosition() const		{ return *mDisplacedPosAttr; }

		float			mRadius = 1.0f;							///< Property: 'Radius' of the sphere
		uint			mRings = 50;							///< Property: 'Rings' number of rings
		uint			mSectors = 50;							///< Property: 'Sectors' number of sectors
		EMemoryUsage	mUsage = EMemoryUsage::Static;			///< Property: 'Usage' If the mesh is uploaded once or frequently updated.
		ECullMode		mCullMode = ECullMode::Back;			///< Property: 'CullMode' controls which triangles are culled, back facing, front facing etc.
		EPolygonMode	mPolygonMode = EPolygonMode::Fill;		///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)
		RGBAColorFloat	mColor = { 1.0f, 1.0f, 1.0f, 1.0f };	///< Property: 'Color' the vertex color of the sphere

	private:
		RenderService* mRenderService;
		std::unique_ptr<MeshInstance> mMeshInstance;

		Vec3VertexAttribute* mOriginalPosAttr  = nullptr;		///< Original vertex positions
		Vec3VertexAttribute* mOriginalNorAttr  = nullptr;		///< Original vertex normals
		Vec3VertexAttribute* mDisplacedPosAttr = nullptr;		///< Displaced vertex positions
	};
}
