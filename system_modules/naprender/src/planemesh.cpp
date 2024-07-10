/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "planemesh.h"
#include "material.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>
#include <nap/numeric.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PlaneMesh, "A grid of x rows and columns, arranged along the X-Y axis in a rectangular pattern")
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::PlaneMesh::mUsage,		nap::rtti::EPropertyMetaData::Default, "If the mesh is static or updated at runtime")
	RTTI_PROPERTY("CullMode",		&nap::PlaneMesh::mCullMode,		nap::rtti::EPropertyMetaData::Default, "Triangle cull mode: front, back, none..")
	RTTI_PROPERTY("PolygonMode",	&nap::PlaneMesh::mPolygonMode,	nap::rtti::EPropertyMetaData::Default, "Polygon draw mode (fill, line, points etc..)")
	RTTI_PROPERTY("Size",			&nap::PlaneMesh::mSize,			nap::rtti::EPropertyMetaData::Default, "Plane size")
	RTTI_PROPERTY("Position",		&nap::PlaneMesh::mPosition,		nap::rtti::EPropertyMetaData::Default, "Plane position")
	RTTI_PROPERTY("Color",			&nap::PlaneMesh::mColor,		nap::rtti::EPropertyMetaData::Default, "Plane vertex color")
	RTTI_PROPERTY("Rows",			&nap::PlaneMesh::mRows,			nap::rtti::EPropertyMetaData::Default, "Number of rows")
	RTTI_PROPERTY("Columns",		&nap::PlaneMesh::mColumns,		nap::rtti::EPropertyMetaData::Default, "Number of columns")
RTTI_END_CLASS
 
namespace nap
{
	PlaneMesh::PlaneMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{ }


	bool PlaneMesh::init(utility::ErrorState& errorState)
	{
		// Setup plane
		if (!setup(errorState))
			return false;

		// Initialize instance
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
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
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
		uint row_vert_count = mRows + 1;
		uint col_vert_count = mColumns + 1;
		uint vert_count = row_vert_count * col_vert_count;

		std::vector<glm::vec3> vertices(vert_count, { 0.0f,0.0f,0.0f });
		std::vector<glm::vec3> normals(vert_count, { 0.0f,0.0f,1.0f });
		std::vector<glm::vec3> uvs(vert_count, { 0.0f,0.0f,0.0f });

		uint idx = 0;
		float min_x = planeRect.getMin().x;
		float min_y = planeRect.getMin().y;

		// Fill vertex uv and position vertex buffers 
		for (uint row = 0; row < row_vert_count; row++)
		{
			// Calculate y values
			float ve_y = min_y + (row * inc_row_v);
			float uv_y = row * inc_row_uv;

			for (int col = 0; col < col_vert_count; col++)
			{
				// Set
				glm::vec3& vertex = vertices[idx];
				vertex.x = min_x + (col * inc_col_v);
				vertex.y = ve_y;

				glm::vec3& uv = uvs[idx];
				uv.x = col * inc_col_uv;
				uv.y = uv_y;

				idx++;
			}
		}

		// Create indices, every cell in the grid contains 2 triangles
		uint triangle_count = mRows * mColumns * 2;
		std::vector<uint32> indices(triangle_count * 3, 0);
		uint32* index_ptr = indices.data();

		for (uint row = 0; row < mRows; row++)
		{
			for (uint col = 0; col < mColumns; col++)
			{
				// Compute triangle a
				*(index_ptr++) = (row * col_vert_count) + col;							//< Bottom Left
				*(index_ptr++) = (row * col_vert_count) + (col + 1);					//< Bottom Right
				*(index_ptr++) = ((row+1) * col_vert_count) + (col + 1);				//< Top right

				// Compute triangle b
				*(index_ptr++) = (row * col_vert_count) + col;							//< Bottom Left
				*(index_ptr++) = ((row + 1) * col_vert_count) + (col + 1);				//< Top right
				*(index_ptr++) = ((row + 1) * col_vert_count) + (col + 0);				//< Top left
			}
		}

		Vec3VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
		Vec3VertexAttribute& normal_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::normal);
		Vec3VertexAttribute& uv_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		Vec4VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

		// Set the number of vertices to use
		mesh.setNumVertices(vert_count);
		mesh.setDrawMode(EDrawMode::Triangles);
		mesh.setUsage(mUsage);
		mesh.setCullMode(mCullMode);
		mesh.setPolygonMode(mPolygonMode);

		// Push vertex data
		position_attribute.setData(vertices.data(), vert_count);
		normal_attribute.setData(normals.data(), vert_count);
		uv_attribute.setData(uvs.data(), vert_count);
		color_attribute.setData({vert_count, mColor.toVec4()});

		MeshShape& shape = mesh.createShape();
		shape.setIndices(indices.data(), indices.size());
	}
};
