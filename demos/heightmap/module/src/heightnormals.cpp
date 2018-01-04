#include "heightnormals.h"
#include "heightmesh.h"

// nap::heightnormals run time class definition 
RTTI_BEGIN_CLASS(nap::HeightNormals)
	// Put additional properties here
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	HeightNormals::~HeightNormals()			{ }


	bool HeightNormals::init(utility::ErrorState& errorState)
	{
		// Make sure the reference mesh is a height mesh
		bool ref_is_height = mReferenceMesh->get_type().is_derived_from(RTTI_OF(nap::HeightMesh));
		if (!errorState.check(ref_is_height, "Reference mesh needs to be a HeightMesh"))
			return false;

		// Setup default normals visualizer, this will create all the default attributes
		// we also need for visualization. We only add 2 other ones to correctly blend 
		// the position of the normals from 0 to 1
		if (!VisualizeNormalsMesh::setup(errorState))
			return false;

		// Get the height mesh, we cast it using rtti_cast, which performs a type
		// check to ensure the cast is allowed, ie: the reference mesh needs to be a height mesh
		mHeightMesh = rtti_cast<HeightMesh>(mReferenceMesh.get());
		assert(mHeightMesh != nullptr);

		// Create original positions attribute
		mOriginalPosAttr = &getMeshInstance().GetOrCreateAttribute<glm::vec3>("OriginalPosition");

		// Get the number of vertices in the height mesh
		int vert_count = mHeightMesh->getMeshInstance().getNumVertices();

		// Get the reference vertices and normals
		std::vector<glm::vec3>& ref_vertices = mHeightMesh->getOriginalPosition().getData();
		std::vector<glm::vec3>& ref_normals  = mHeightMesh->getOriginalNormals().getData();
		
		// Create target buffers, the normals are drawn using lines. Every line has 2 vertices
		// We therefore create a buffer that contains twice the amount of vertices of the reference mesh
		// Every vertex 'receives' an associated normal line
		std::vector<glm::vec3> target_vertices(ref_vertices.size() * 2, { 0.0f, 0.0f, 0.0f });

		// Use those to populate the two new attributes
		int target_idx = 0;
		for (int i = 0; i < vert_count; i++)
		{
			// get current position and normal
			const glm::vec3& ref_vertex = ref_vertices[i];
			const glm::vec3& ref_normal = ref_normals[i];

			// Ensure the normal has length
			assert(glm::length(ref_normal) > 0);

			// Set the original vertex position
			target_vertices[target_idx] = ref_vertex;
			target_vertices[target_idx + 1] = ref_vertex + (glm::normalize(ref_normal) * mNormalLength);

			// Increment write index
			target_idx += 2;
		}

		// Set the attribute data
		mOriginalPosAttr->setData(target_vertices);

		// Initialize our mesh -> create all attributes on the gpu and push data
		return getMeshInstance().init(errorState);
	}
}