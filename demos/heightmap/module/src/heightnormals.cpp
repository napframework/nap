// Local Includes
#include "heightnormals.h"
#include "heightmesh.h"

// External Includes
#include <nap/core.h>

// nap::heightnormals run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::HeightNormals)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	HeightNormals::HeightNormals(nap::Core& core) : VisualizeNormalsMesh(core)
	{ }


	bool HeightNormals::init(utility::ErrorState& errorState)
	{
		// Make sure the reference mesh is a height mesh
		bool ref_is_height = mReferenceMesh->get_type().is_derived_from(RTTI_OF(nap::HeightMesh));
		if (!errorState.check(ref_is_height, "Reference mesh needs to be a HeightMesh"))
			return false;

		// Create the normals mesh instance that we want to populate below
		if (!createMeshInstance(errorState))
			return false;

		// Setup default normals visualizer, this will create all the default attributes
		// we also need for visualization. We only add 2 other ones to correctly blend 
		// the position of the normals from 0 to 1
		if (!setReferenceMesh(*mReferenceMesh, errorState))
			return false;

		// Calculate our normals
		if (!calculateNormals(errorState, false))
			return false;

		// Get the height mesh, we cast it using rtti_cast, which performs a type
		// check to ensure the cast is allowed, ie: the reference mesh needs to be a height mesh
		mHeightMesh = rtti_cast<HeightMesh>(mReferenceMesh.get());
		assert(mHeightMesh != nullptr);

		// Create original positions attribute
		mOriginalPosAttr = &getMeshInstance().getOrCreateAttribute<glm::vec3>("OriginalPosition");
		mOriginalNorAttr = &getMeshInstance().getOrCreateAttribute<glm::vec3>("DisplacedPosition");

		// Get the number of vertices in the height mesh
		int vert_count = mHeightMesh->getMeshInstance().getNumVertices();

		// Get the reference vertices and normals
		VertexAttribute<glm::vec3>& ref_original_vertices = mHeightMesh->getOriginalPosition();
		VertexAttribute<glm::vec3>& ref_original_normals = mHeightMesh->getOriginalNormals();
		VertexAttribute<glm::vec3>& ref_displaced_vertices = mHeightMesh->getDisplacedPosition();

		// Create target buffers, the normals are drawn using lines. Every line has 2 vertices
		// We therefore create a buffer that contains twice the amount of vertices of the reference mesh
		// Every vertex 'receives' an associated normal line
		std::vector<glm::vec3> original_vertices(ref_original_vertices.getCount()  * 2, { 0.0f, 0.0f, 0.0f });
		std::vector<glm::vec3> displaced_vertices(ref_original_vertices.getCount() * 2, { 0.0f, 0.0f, 0.0f });

		// Use those to populate the two new attributes
		int target_idx = 0;
		for (int i = 0; i < vert_count; i++)
		{
			// get current position and normal
			const glm::vec3& ref_ori_vertex = ref_original_vertices[i];
			const glm::vec3& ref_ori_normal = ref_original_normals[i];
			const glm::vec3& ref_displa = ref_displaced_vertices[i];

			// Ensure the normal has length
			assert(glm::length(ref_ori_normal) > 0);

			// Set the original vertex positions for the normal
			original_vertices[target_idx] = ref_ori_vertex;
			original_vertices[target_idx + 1] = ref_ori_vertex + (glm::normalize(ref_ori_normal) * mNormalLength);

			// Set the displaced vertex positions for the normal. We want to show in the viewport
			// the normal at the right displaced position pointing in the direction of the
			// original non-displaced normal
			displaced_vertices[target_idx] = ref_displa;
			displaced_vertices[target_idx + 1] = ref_displa + (glm::normalize(ref_ori_normal) * mNormalLength);

			// Increment write index
			target_idx += 2;
		}

		// Set the attribute data
		mOriginalPosAttr->setData(original_vertices);
		mOriginalNorAttr->setData(displaced_vertices);

		// Initialize our mesh -> create all attributes on the gpu and push data
		return getMeshInstance().init(errorState);
	}
}
