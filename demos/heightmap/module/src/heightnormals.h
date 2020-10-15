/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/object.h>
#include <visualizenormalsmesh.h>
#include <heightmesh.h>

namespace nap
{
	/**
	 * A mesh resources that computes a line (normal) for every vertex in a height mesh.
	 * The normals can be displayed using a RenderableMesh and material.
	 * On initialization this object creates all the mesh data associated with it's parent: VisualizeNormalsMesh
	 * The result of that call is a mesh that can be used to display the normals of a fully displaced height mesh.
	 * This resource also adds 2 extra vertex attributes: "OriginalPosition" and "DisplacedPosition".
	 * Original position holds the vertex positions associated with the non-displaced height map.
	 * Displaced position holds the displaced vertex position that points in the original non-displaced direction
	 * Both can be used to visualize how the normals change when the height map blend value is adjusted
	 * See VisualizeNormalsMesh for more information.
	 */
	class HeightNormals : public VisualizeNormalsMesh
	{
		RTTI_ENABLE(VisualizeNormalsMesh)
	public:
		HeightNormals(nap::Core& core);

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		HeightMesh* mHeightMesh = nullptr;						///< Pointer to the normals we want to visualize
		Vec3VertexAttribute* mOriginalPosAttr  = nullptr;		///< Original vertex positions
		Vec3VertexAttribute* mDisplacedPosAttr = nullptr;		///< Displaced but not blended normal direction
	};
}