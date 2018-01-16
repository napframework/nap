#include "planemesh.h"
#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>


RTTI_BEGIN_CLASS(nap::PlaneMesh)
	RTTI_PROPERTY("Size",		&nap::PlaneMesh::mSize,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Position",	&nap::PlaneMesh::mPosition, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Rows",		&nap::PlaneMesh::mRows,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Columns",	&nap::PlaneMesh::mColumns,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


namespace nap
{
	bool PlaneMesh::init(utility::ErrorState& errorState)
	{
		if (!setup(errorState))
			return false;

		// Initialize mesh
		return mMeshInstance->init(errorState);
	}


	bool PlaneMesh::setup(utility::ErrorState& error)
	{
		// Make sure number of rows and columns is > 0
		if (!error.check(mRows > 0, "Invalid number of rows, needs to be higher than 0: %s", this->mID.c_str()))
			return false;

		if (!error.check(mColumns > 0, "Invalid number of columns, needs to be higher than 0: %s", this->mID.c_str()))
			return false;

		// Construct bounding rect
		float dsizex = (0.0f - (mSize.x / 2.0f)) + mPosition.x;
		float dsizey = (0.0f - (mSize.y / 2.0f)) + mPosition.y;
		math::Rect rect(dsizex, dsizey, mSize.x, mSize.y);

		// Create plane
		mMeshInstance = std::make_unique<MeshInstance>();
		constructPlane(rect, *mMeshInstance);

		// Store rect
		mRect = rect;

		return true;
	}


	void PlaneMesh::constructPlane(const math::Rect& planeRect, nap::MeshInstance& mesh)
	{
		// Get incremental stepping values
		float inc_row_v = planeRect.getHeight() / static_cast<float>(mRows);
		float inc_col_v = planeRect.getWidth()  / static_cast<float>(mColumns);

		float inc_row_uv = 1.0f / static_cast<float>(mRows);
		float inc_col_uv = 1.0f / static_cast<float>(mColumns);

		// Create buffers for all attributes
		int row_vert_count = mRows + 1;
		int col_vert_count = mColumns + 1;
		int vert_count = row_vert_count * col_vert_count;

		std::vector<glm::vec3> vertices(vert_count, { 0.0f,0.0f,0.0f });
		std::vector<glm::vec3> normals(vert_count, { 0.0f,0.0f,1.0f });
		std::vector<glm::vec3> uvs(vert_count, { 0.0f,0.0f,0.0f });
		std::vector<glm::vec4> colors(vert_count, { 1.0f, 1.0f, 1.0f, 1.0f });

		int idx = 0;
		float min_x = planeRect.getMin().x;
		float min_y = planeRect.getMin().y;

		// Fill vertex uv and position vertex buffers 
		for (int row = 0; row < row_vert_count; row++)
		{
			// Calculate y values
			float ve_y = min_y + (row * inc_row_v);
			float uv_y = row * inc_row_uv;

			for (int col = 0; col < col_vert_count; col++)
			{
				// Set
				vertices[idx].x = min_x + (col * inc_col_v);
				vertices[idx].y = ve_y;

				uvs[idx].x = col * inc_col_uv;
				uvs[idx].y = uv_y;

				idx++;
			}
		}

		// Create indices, every cell in the grid contains 2 triangles
		int triangle_count = mRows * mColumns * 2;
		std::vector<unsigned int> indices(triangle_count * 3, 0);
		auto it = indices.begin();

		for (int row = 0; row < mRows; row++)
		{
			for (int col = 0; col < mColumns; col++)
			{
				// Compute triangle a
				*(it++) = (row * col_vert_count) + col;							//< Bottom Left
				*(it++) = (row * col_vert_count) + (col + 1);					//< Bottom Right
				*(it++) = ((row+1) * col_vert_count) + (col + 1);				//< Top right

				// Compute triangle b
				*(it++) = (row * col_vert_count) + col;							//< Bottom Left
				*(it++) = ((row + 1) * col_vert_count) + (col + 1);				//< Top right
				*(it++) = ((row + 1) * col_vert_count) + (col + 0);				//< Top left
			}
		}

		Vec3VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		Vec3VertexAttribute& normal_attribute = mesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());
		Vec3VertexAttribute& uv_attribute = mesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		Vec4VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));

		// Set the number of vertices to use
		mesh.setNumVertices(vert_count);

		// Push vertex data
		position_attribute.setData(vertices.data(), vert_count);
		normal_attribute.setData(normals.data(), vert_count);
		uv_attribute.setData(uvs.data(), vert_count);
		color_attribute.setData(colors.data(), vert_count);

		SubMesh& sub_mesh = mesh.createSubMesh();
		sub_mesh.setDrawMode(opengl::EDrawMode::TRIANGLES);
		sub_mesh.setIndices(indices.data(), indices.size());
	}

};