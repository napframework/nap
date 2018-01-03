#include "heightmesh.h"

// External Includes
#include <meshutils.h>
#include <nap/logger.h>

// nap::heightmesh run time class definition 
RTTI_BEGIN_CLASS(nap::HeightMesh)
	RTTI_PROPERTY("Heightmap",	&nap::HeightMesh::mHeightmap,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Height",		&nap::HeightMesh::mHeight,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	HeightMesh::~HeightMesh()			{ }


	bool HeightMesh::init(utility::ErrorState& errorState)
	{
		// Initialize our plane with the right amount of rows / columns
		if (!PlaneMesh::init(errorState))
			return false;

		// Get the just created and initialized mesh instance
		nap::MeshInstance& mesh = getMeshInstance();

		// Get attributes
		Vec3VertexAttribute& pos_attr = mesh.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
		Vec3VertexAttribute& uvs_attr = mesh.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
		Vec3VertexAttribute& nor_attr = mesh.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::getNormalName());

		std::vector<glm::vec3>& uvs_data = uvs_attr.getData();
		std::vector<glm::vec3>& pos_data = pos_attr.getData();
		std::vector<glm::vec3>& nor_data = nor_attr.getData();

		// Get the pixmap we want to get the color value from @uv
		const nap::Pixmap& pixmap = mHeightmap->getPixmap();
		float width = static_cast<float>(pixmap.getWidth() - 1);
		float heigh = static_cast<float>(pixmap.getHeight() - 1);

		int vert_count = mesh.getNumVertices();
		for (int i = 0; i < vert_count; i++)
		{
			// Get current vertex uv data
			glm::vec3& uvs = uvs_data[i];

			// Get current vector normal 
			glm::vec3& nor = nor_data[i];

			// Calculate pixel coordinate
			int pixel_x = static_cast<int>(uvs.x * width);
			int pixel_y = static_cast<int>(uvs.y * heigh);

			// Get our color, this also converts it to a float value
			RColorFloat color = pixmap.getColor<RColorFloat>(pixel_x, pixel_y);

			// Set the new vertex position
			pos_data[i] = pos_data[i] + (nor * color.getRed() * mHeight);
		}

		// Update our mesh normals to ensure light calculations work in the shader
		computeNormals(mesh, pos_attr, nor_attr);

		// Now push changes
		nap::utility::ErrorState error;
		if (!mesh.update(error))
			return false;
		return true;

	}
}
