#include "meshutils.h"
#include <mathutils.h>
#include <glm/gtx/normal.hpp>
#include "triangleiterator.h"

namespace nap
{
	bool NAPAPI isTriangleMesh(const MeshShape& shape)
	{
		switch (shape.getDrawMode())
		{
		case opengl::EDrawMode::LINE_LOOP:
		case opengl::EDrawMode::LINE_STRIP:
		case opengl::EDrawMode::LINES:
		case opengl::EDrawMode::POINTS:
		case opengl::EDrawMode::UNKNOWN:
			return false;
		case opengl::EDrawMode::TRIANGLES:
		case opengl::EDrawMode::TRIANGLE_FAN:
		case opengl::EDrawMode::TRIANGLE_STRIP:
			return true;
		default:
			assert(false);
			return false;
		}
	}
	

	void NAPAPI setTriangleIndices(MeshShape& mesh, int number, glm::ivec3& indices)
	{		
		// Copy triangle index over
		MeshShape::IndexList& mesh_indices = mesh.getIndices();

		switch (mesh.getDrawMode())
		{
		case opengl::EDrawMode::TRIANGLES:
		{
			// Make sure our index is in range
			assert((number * 3) + 2 < mesh_indices.size());

			// Fill the data
			unsigned int* id = mesh_indices.data() + (number * 3);
			*(id + 0) = indices.x;
			*(id + 1) = indices.y;
			*(id + 2) = indices.z;
			break;
		}
		case opengl::EDrawMode::TRIANGLE_FAN:
		{
			assert(number + 2 < mesh_indices.size());
			unsigned int* id = mesh_indices.data();
			*id = indices.x;
			*(id + number + 1) = indices.y;
			*(id + number + 2) = indices.z;
			break;
		}
		case opengl::EDrawMode::TRIANGLE_STRIP:
		{
			assert(number + 2 < mesh_indices.size());
			unsigned int* id = mesh_indices.data() + number;
			*(id + 0) = indices.x;
			*(id + 1) = indices.y;
			*(id + 2) = indices.z;
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void computeBoundingBox(const MeshInstance& mesh, math::Box& outBox)
	{
		glm::vec3 min = { nap::math::max<float>(), nap::math::max<float>(), nap::math::max<float>() };
		glm::vec3 max = { nap::math::min<float>(), nap::math::min<float>(), nap::math::min<float>() };

		const nap::VertexAttribute<glm::vec3>& positions = mesh.getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		for (const auto& point : positions.getData())
		{
			if (point.x < min.x) { min.x = point.x; }
			if (point.x > max.x) { max.x = point.x; }
			if (point.y < min.y) { min.y = point.y; }
			if (point.y > max.y) { max.y = point.y; }
			if (point.z < min.z) { min.z = point.z; }
			if (point.z > max.z) { max.z = point.z; }
		}
		outBox.mMinCoordinates = min;
		outBox.mMaxCoordinates = max;
	}


	nap::math::Box computeBoundingBox(const MeshInstance& mesh)
	{
		math::Box box;
		computeBoundingBox(mesh, box);
		return box;
	}


	void computeNormals(const MeshInstance& meshInstance, const VertexAttribute<glm::vec3>& positions, VertexAttribute<glm::vec3>& outNormals)
	{
		assert(outNormals.getCount() == positions.getCount());
		
		// Total number of attributes
		int attr_count = positions.getCount();

		// Normal data
		const std::vector<glm::vec3>& position_data = positions.getData();
		std::vector<glm::vec3>& normal_data = outNormals.getData();
		
		// Reset normal data so we can accumulate data into it. Note that this is a memset
		// instead of a loop to improve performance
		std::memset(normal_data.data(), 0, sizeof(glm::vec3) * normal_data.size());

		// Accumulate all normals into the normals array
		const glm::vec3* position_data_ptr = position_data.data();
		glm::vec3* normal_data_ptr = normal_data.data();

		// Go over all triangles in all triangle shapes
		TriangleShapeIterator iterator(meshInstance);
		while (!iterator.isDone())
		{
			glm::ivec3 indices = iterator.next();

			const glm::vec3& point0 = position_data_ptr[indices[0]];
			const glm::vec3& point1 = position_data_ptr[indices[1]];
			const glm::vec3& point2 = position_data_ptr[indices[2]];

			glm::vec3 normal = glm::cross((point0 - point1), (point0 - point2));

			normal_data_ptr[indices[0]] += normal;
			normal_data_ptr[indices[1]] += normal;
			normal_data_ptr[indices[2]] += normal;
		}

		// Normalize to deal with shared vertices
		for (glm::vec3& normal : normal_data)
			normal = glm::normalize(normal);
	}


	void NAPAPI reverseWindingOrder(MeshInstance& mesh)
	{
		TriangleShapeIterator iterator(mesh);
		while (!iterator.isDone())
		{
			glm::ivec3 cindices = iterator.next();
			std::swap(cindices.x, cindices.z);

			int shape_index = iterator.getCurrentShapeIndex();
			int triangle_index = iterator.getCurrentTriangleIndex();
			MeshShape& shape = mesh.getShape(shape_index);

			setTriangleIndices(shape, triangle_index, cindices);
		}
	}


	void NAPAPI generateIndices(nap::MeshShape& shape, int vertexCount, int offset)
	{
		MeshShape::IndexList& indices = shape.getIndices();
		indices.resize(vertexCount);
		for (int vertex = 0; vertex < vertexCount; ++vertex)
			indices[vertex] = vertex + offset;
	}
}