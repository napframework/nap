#include "visualizenormalsmesh.h"
#include <rtti/rtti.h>

RTTI_BEGIN_CLASS(nap::VisualizeNormalsMesh)
	RTTI_PROPERTY("ReferenceMesh", &nap::VisualizeNormalsMesh::mReferenceMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length", &nap::VisualizeNormalsMesh::mNormalLength, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool VisualizeNormalsMesh::init(utility::ErrorState& errorState)
	{
		// Create the mesh that will hold the normals
		mMeshInstance = std::make_unique<MeshInstance>();

		nap::IMesh* reference_mesh = mReferenceMesh.get();
		
		// Make sure the reference mesh has normals
		if (reference_mesh->getMeshInstance().findAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName()) == nullptr)
			return errorState.check(false, "reference mesh has no normals");

		// Create position and vertex attribute
		mPositionAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName()));

		// Create color attribute
		mColorAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0)));

		int vertex_count = reference_mesh->getMeshInstance().getNumVertices();

		// Create initial color data
		std::vector<glm::vec4> colors(vertex_count * 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		mColorAttr->setData(colors);

		// Create initial position data
		std::vector<glm::vec3> vertices(vertex_count * 2, { 0.0f, 0.0f, 0.0f });
		mPositionAttr->setData(vertices);

		// Set number of vertices
		mMeshInstance->setNumVertices(vertex_count * 2);

		// Update normals
		updateNormals(errorState, false);

		if (!mMeshInstance->init(errorState))
			return false;

		// Draw normals as lines
		mMeshInstance->setDrawMode(opengl::EDrawMode::LINES);

		return true;
	}


	bool VisualizeNormalsMesh::updateNormals(utility::ErrorState& error, bool push)
	{
		const nap::MeshInstance& reference_mesh = mReferenceMesh.get()->getMeshInstance();

		// Get reference normals and vertices
		const std::vector<glm::vec3>& ref_normals  = reference_mesh.getAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName()).getData();
		const std::vector<glm::vec3>& ref_vertices = reference_mesh.getAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName()).getData();
		
		// Try to find a color attribute to pass along
		const Vec4VertexAttribute* ref_color_attr = reference_mesh.findAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));
		const std::vector<glm::vec4>* ref_colors = ref_color_attr != nullptr ? &(ref_color_attr->getData()) : nullptr;

		// Get buffers to populate
		std::vector<glm::vec3>& target_vertices = mPositionAttr->getData();
		std::vector<glm::vec4>& target_colors = mColorAttr->getData();

		int vert_count = reference_mesh.getNumVertices();
		int target_idx = 0;

		glm::vec4 bottom_color(1.0f, 1.0f, 1.0f, 0.0f);
		glm::vec4 top_color(1.0f, 1.0f, 1.0f, 1.0f);

		for (int i = 0; i < vert_count; i++)
		{
			// get current position and normal
			const glm::vec3& ref_vertex = ref_vertices[i];
			const glm::vec3& ref_normal = ref_normals[i];

			// Ensure the normal has length
			assert(glm::length(ref_normal) > 0);

			// Set vertices
			target_vertices[target_idx] = ref_vertex;
			target_vertices[target_idx + 1] = ref_vertex + (glm::normalize(ref_normal) * mNormalLength);

			// Set the color based on the vert color, otherwise remains the same
			bottom_color.r = ref_colors != nullptr ? (*ref_colors)[i].r : 1.0f;
			bottom_color.g = ref_colors != nullptr ? (*ref_colors)[i].g : 1.0f;
			bottom_color.b = ref_colors != nullptr ? (*ref_colors)[i].b : 1.0f;

			// Set the top color to have the same color (saves a sample step)
			top_color.r = bottom_color.r;
			top_color.g = bottom_color.g;
			top_color.b = bottom_color.b;

			target_colors[target_idx] = top_color;
			target_colors[target_idx + 1] = bottom_color;

			// Increment write index
			target_idx += 2;
		}

		if (push)
		{
			return mMeshInstance->update(error);
		}
		return true;
	}

}