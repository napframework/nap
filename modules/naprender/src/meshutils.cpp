#include "meshutils.h"
#include <mathutils.h>

namespace nap
{
	bool NAPAPI hasIndices(const MeshInstance& mesh)
	{
		return mesh.hasIndices();
	}


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


	void getBoundingBox(const MeshInstance& mesh, math::Box& outBox)
	{
		glm::vec3 min = { nap::math::max<float>(), nap::math::max<float>(), nap::math::max<float>() };
		glm::vec3 max = { nap::math::min<float>(), nap::math::min<float>(), nap::math::min<float>() };

		const nap::VertexAttribute<glm::vec3>& positions = mesh.GetAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
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


	nap::math::Box getBoundingBox(const MeshInstance& mesh)
	{
		math::Box box;
		getBoundingBox(mesh, box);
		return box;
	}

}