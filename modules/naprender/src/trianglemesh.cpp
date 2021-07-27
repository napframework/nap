/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "trianglemesh.h"
#include "mesh.h"
#include "material.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <glm/glm.hpp>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TriangleMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS


namespace nap
{
	TriangleMesh::TriangleMesh(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}

	bool TriangleMesh::init(utility::ErrorState& errorState)
	{
		if (!setup(errorState))
			return false;

		// Initialize mesh
		return mMeshInstance->init(errorState);
	}


	bool TriangleMesh::setup(utility::ErrorState& error)
	{
		// Create plane
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
		constructTriangle(*mMeshInstance);
		return true;
	}


	void TriangleMesh::constructTriangle(nap::MeshInstance& mesh)
	{
		std::vector<glm::vec2> vertices = {
			{  0.0f, -0.5f },
			{  0.5f,  0.5f },
			{ -0.5f,  0.5f },
			{  1.0f,  1.0f }
		};

		std::vector<glm::vec3> colors = {
			{ 1.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f },
			{ 1.0f, 1.0f, 1.0f }
		};

		std::vector<uint32> indices = {
			0, 1, 2, 3
		};

		Vec2VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec2>(vertexid::position);
		Vec3VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getColorName(0));

		// Set the number of vertices to use
		mesh.setNumVertices(vertices.size());
		mesh.setDrawMode(EDrawMode::TriangleStrip);
		mesh.setCullMode(ECullMode::Back);
		mesh.setUsage(EMeshDataUsage::Static);
		mesh.setPolygonMode(EPolygonMode::Fill);

		// Push vertex data
		position_attribute.setData(vertices.data(), vertices.size());
		color_attribute.setData(colors.data(), vertices.size());

		MeshShape& shape = mesh.createShape();
		shape.setIndices(indices.data(), indices.size());
	}
};
