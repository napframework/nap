/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once
#include "mesh.h"
#include "triangleiterator.h"

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declares
	class Core;
	class RenderService;

	/**
	 * Randomly distributes a certain amount of points over the surface of a mesh
	 * The result is a new mesh that does not contain any triangles, only points as vertices
	 * The uv, color and normal attributes are copied over from the reference mesh if available!
	 */
	class NAPAPI ScatterPointsMesh : public IMesh
	{
		RTTI_ENABLE(IMesh)
	public:
		ScatterPointsMesh(Core& core);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the scattered point mesh
		 */
		virtual MeshInstance& getMeshInstance() override						{ return *mMeshInstance;  }

		/**
		 * @return the scattered point mesh
		 */
		virtual const MeshInstance& getMeshInstance() const override			{ return *mMeshInstance; }

		ResourcePtr<IMesh> mReferenceMesh = nullptr;							///< Property: 'ReferenceMesh' link to the mesh that is used as a reference, can be null (ie: nothing)
		int mNumberOfPoints = 1000;												///< Property: 'PointCount' number of points to scatter on the mesh
		EMemoryUsage	mUsage = EMemoryUsage::Static;						///< Property: 'Usage' If the plane is created once or frequently updated.

	protected:
		// Creates the mesh and shape
		bool createMeshInstance(nap::utility::ErrorState& error);

		// Sets up the mesh instance based on the total amount of points to scatter
		bool setup(nap::utility::ErrorState& error);

		// Scatter points
		bool scatterPoints(nap::utility::ErrorState& error);

		//Helper function that computes the total visible area of the mesh
		using TriangleAreaMap = std::map<float, Triangle>;
		float computeArea(TriangleAreaMap& outMap);

		std::unique_ptr<MeshInstance> mMeshInstance = nullptr;					///< The actual mesh that contains all the points
		nap::Vec3VertexAttribute* mPositionAttr = nullptr;						///< Position Attribute data
		nap::Vec3VertexAttribute* mNormalsAttr = nullptr;						///< Normal attribute data
		std::vector<nap::Vec3VertexAttribute*> mUvAttrs;						///< UV attribute data
		std::vector<nap::Vec4VertexAttribute*> mColorAttrs;						///< Color attribute data
		RenderService* mRenderService = nullptr;								///< Handle to render service
	};
}
