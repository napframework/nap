#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <visualizenormalsmesh.h>
#include <heightmesh.h>

namespace nap
{
	/**
	 * Creates height map normals based on the data in a HeightMesh.
	 * The normals an be displayed using a RenderableMesh and material.
	 * On initialization this object first creates all the mesh data associated with it's parent: VisualizeNormalsMesh 
	 * After that it adds an extra vertex attribute: "OriginalPosition". This attribute is used to
	 * blend the position of the normals in a mesh based on the shader blend value.
	 * This ensures the normals 'track' the height map when the blend value changes
	 * See VisualizeNormalsMesh for more information.
	 */
	class HeightNormals : public VisualizeNormalsMesh
	{
		RTTI_ENABLE(VisualizeNormalsMesh)
	public:
		virtual ~HeightNormals();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

	private:
		HeightMesh* mHeightMesh = nullptr;						///< Pointer to the normals we want to visualize
		Vec3VertexAttribute* mOriginalPosAttr = nullptr;		///< Original vertex positions
	};
}