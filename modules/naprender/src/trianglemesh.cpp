#include "TriangleMesh.h"
#include "mesh.h"
#include "material.h"
#include <glm/glm.hpp>
#include "renderservice.h"
#include "nap/core.h"

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

		Vec2VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec2>(VertexAttributeIDs::getPositionName());
		Vec3VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::GetColorName(0));

		// Set the number of vertices to use
		mesh.setNumVertices(vertices.size());
		mesh.setDrawMode(EDrawMode::TriangleStrip);
		mesh.setCullMode(ECullMode::Back);
		mesh.setUsage(EMeshDataUsage::Static);

		// Push vertex data
		position_attribute.setData(vertices.data(), vertices.size());
		color_attribute.setData(colors.data(), vertices.size());

		MeshShape& shape = mesh.createShape();
		shape.setIndices(indices.data(), indices.size());
	}
};