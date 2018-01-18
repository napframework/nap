#include "meshutils.h"
#include <mathutils.h>
#include <glm/gtx/normal.hpp>

namespace nap
{
	bool NAPAPI isTriangleMesh(const MeshInstance& mesh)
	{
		switch (mesh.getDrawMode())
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


	int NAPAPI getTriangleCount(const MeshInstance& mesh)
	{
		switch(mesh.getDrawMode())
		{
		case opengl::EDrawMode::TRIANGLES:
		{
			if (mesh.hasIndices())
			{
				assert(mesh.getIndices().size() % 3 == 0);
				return mesh.getIndices().size() / 3;
			}
			assert(mesh.getNumVertices() % 3 == 0);
			return mesh.getNumVertices() / 3;
		}
		case opengl::EDrawMode::TRIANGLE_FAN:		// Fan and strip need at least 3 vertices to make up 1 triangle. 
		case opengl::EDrawMode::TRIANGLE_STRIP:		// After that every vertex is a triangle
		{
			if (mesh.hasIndices())
			{
				return math::max<int>(mesh.getIndices().size() - 2,0);
			}
			return math::max<int>(mesh.getNumVertices() -2,0);
		}
		case opengl::EDrawMode::LINE_LOOP:
		case opengl::EDrawMode::LINE_STRIP:
		case opengl::EDrawMode::LINES:
		case opengl::EDrawMode::POINTS:
		case opengl::EDrawMode::UNKNOWN:
			return -1;
		default:
			assert(false);
			return -1;
		}
	}


	void NAPAPI getTriangleIndices(const MeshInstance& mesh, int number, glm::ivec3& indices)
	{		
		// Make sure the index is valid
		assert(mesh.hasIndices());

		// Copy triangle index over
		const std::vector<uint>& mesh_indices = mesh.getIndices();

		switch (mesh.getDrawMode())
		{
		case opengl::EDrawMode::TRIANGLES:
		{
			// Make sure our index is in range
			assert((number * 3) + 2 < mesh_indices.size());

			// Fill the data
			const unsigned int* id = mesh_indices.data() + (number * 3);
			indices.x = *(id + 0);
			indices.y = *(id + 1);
			indices.z = *(id + 2);
			break;
		}
		case opengl::EDrawMode::TRIANGLE_FAN:
		{
			assert(number + 2 < mesh_indices.size());
			const unsigned int* id = mesh_indices.data();
			indices.x = *id;
			indices.y = *(id + number + 1);
			indices.z = *(id + number + 2);
			break;
		}
		case opengl::EDrawMode::TRIANGLE_STRIP:
		{
			assert(number + 2 < mesh_indices.size());
			const unsigned int* id = mesh_indices.data() + number;
			indices.x = *(id + 0);
			indices.y = *(id + 1);
			indices.z = *(id + 2);
			break;
		}
		default:
			assert(false);
			break;
		}
	}


	void NAPAPI setTriangleIndices(MeshInstance& mesh, int number, glm::ivec3& indices)
	{		
		// Make sure the index is valid
		assert(mesh.hasIndices());

		// Copy triangle index over
		std::vector<uint>& mesh_indices = mesh.getIndices();

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


	glm::vec3 computeTriangleNormal(const nap::MeshInstance& mesh, int number, const nap::VertexAttribute<glm::vec3>& vertices)
	{
		TriangleData<glm::vec3> triangle_data;
		getTriangleValues<glm::vec3>(mesh, number, vertices, triangle_data);
		return glm::cross((triangle_data[0] - triangle_data[1]), (triangle_data[0] - triangle_data[2]));
	}


	glm::vec3 computePointNormal(const MeshInstance& mesh, int index, const nap::VertexAttribute<glm::vec3>& vertices, const MeshConnectivityMap& connectivityMap)
	{
		assert(mesh.hasIndices());
		assert(index < connectivityMap.size());
		const std::vector<int>& triangles = connectivityMap[index];

		glm::vec3 point_normal(0.0f, 0.0f, 0.0f);
		for (const auto& triangle : triangles)
		{
			point_normal += computeTriangleNormal(mesh, triangle, vertices);
		}
		return glm::normalize(point_normal);
	}


	void computeNormals(const MeshInstance& mesh, const VertexAttribute<glm::vec3>& vertices, VertexAttribute<glm::vec3>& outNormals)
	{
		assert(outNormals.getCount() == vertices.getCount());
		
		// Total number of attributes
		int attr_count = vertices.getCount();

		// Normal data
		std::vector<glm::vec3>& normal_data = outNormals.getData();

		// Compute normals if the mesh doesn't use indices
		// Result = triangle normal for all triangle vertices
		if (!mesh.hasIndices())
		{
			// Cache current triangle and computed value
			// Lot faster not having to re-compute face normal
			int triangle = -1;
			glm::vec3 current_tri_normal;
			for (int i = 0; i < attr_count; i++)
			{
				if (i/3 != triangle)
				{
					triangle = i/3;
					current_tri_normal = glm::normalize(computeTriangleNormal(mesh, triangle, vertices));
				}
				normal_data[i] = current_tri_normal;
			}
		}
		else
		{
			// Otherwise use connectivity
			nap::MeshConnectivityMap map;
			computeConnectivity(mesh, map);
			for (int i = 0; i < attr_count; i++)
			{
				glm::vec3 normal = computePointNormal(mesh, i, vertices, map);
				normal_data[i] = normal;
			}
		}
	}


	void NAPAPI reverseWindingOrder(MeshInstance& mesh)
	{
		assert(isTriangleMesh(mesh));
		assert(mesh.hasIndices());
		int tri_count = getTriangleCount(mesh);
		glm::ivec3 cindices;
		for (int i = 0; i < tri_count; i++)
		{
			getTriangleIndices(mesh, i, cindices);
			int x = cindices.x;
			cindices.x = cindices.z;
			cindices.z = x;
			setTriangleIndices(mesh, i, cindices);
		}
	}


	void computeConnectivity(const MeshInstance& mesh, MeshConnectivityMap& outConnectivityMap)
	{
		// When the mesh doesn't use indices this is simple, every vertex belongs to exactly one face
		assert(isTriangleMesh(mesh));
		assert(mesh.hasIndices());

		// Resize to number of indices
		outConnectivityMap.resize(mesh.getNumVertices());

		// Get number of triangles
		int triangle_count = getTriangleCount(mesh);

		// For every triangle, fetch the indices
		// Every index is associated with that triangle, so add the triangle to the right index
		glm::ivec3 triangle_indices;
		for (int t = 0; t < triangle_count; t++)
		{
			getTriangleIndices(mesh, t, triangle_indices);
			outConnectivityMap[triangle_indices[0]].emplace_back(t);
			outConnectivityMap[triangle_indices[1]].emplace_back(t);
			outConnectivityMap[triangle_indices[2]].emplace_back(t);
		}
	}
}