#include "meshutils.h"
#include <mathutils.h>
#include <glm/gtx/normal.hpp>
#include <triangleiterator.h>

namespace nap
{
	namespace utility
	{
		bool isTriangleMesh(const MeshInstance& meshInstance)
		{
			switch (meshInstance.getDrawMode())
			{
			case EDrawMode::LineLoop:
			case EDrawMode::LineStrip:
			case EDrawMode::Lines:
			case EDrawMode::Points:
			case EDrawMode::Unknown:
				return false;
			case EDrawMode::Triangles:
			case EDrawMode::TriangleFan:
			case EDrawMode::TriangleStrip:
				return true;
			default:
				assert(false);
				return false;
			}
		}


		int getTriangleCount(const MeshInstance& mesh)
		{
			int count = 0;

			for (int shape_index = 0; shape_index < mesh.getNumShapes(); ++shape_index)
			{
				const MeshShape& shape = mesh.getShape(shape_index);
				switch (mesh.getDrawMode())
				{
				case EDrawMode::Triangles:
				{
					count += shape.getNumIndices() / 3;
					break;
				}
				case EDrawMode::TriangleFan:		// Fan and strip need at least 3 vertices to make up 1 triangle. 
				case EDrawMode::TriangleStrip:		// After that every vertex is a triangle
				{
					count += math::max<int>(shape.getNumIndices() - 2, 0);
					break;
				}
				case EDrawMode::LineLoop:
				case EDrawMode::LineStrip:
				case EDrawMode::Lines:
				case EDrawMode::Points:
				case EDrawMode::Unknown:
					break;
				default:
					assert(false);
					break;
				}
			}

			return count;
		}


		glm::vec3 computeTriangleNormal(const TriangleData<glm::vec3>& vertices)
		{
			return glm::cross((vertices[0] - vertices[1]), (vertices[0] - vertices[2]));
		}


		void  setTriangleIndices(MeshShape& mesh, EDrawMode drawMode, int number, const std::array<int, 3>& indices)
		{
			// Copy triangle index over
			MeshShape::IndexList& mesh_indices = mesh.getIndices();

			switch (drawMode)
			{
			case EDrawMode::Triangles:
			{
				// Make sure our index is in range
				assert((number * 3) + 2 < mesh_indices.size());

				// Fill the data
				unsigned int* id = mesh_indices.data() + (number * 3);
				*(id + 0) = indices[0];
				*(id + 1) = indices[1];
				*(id + 2) = indices[2];
				break;
			}
			case EDrawMode::TriangleFan:
			{
				assert(number + 2 < mesh_indices.size());
				unsigned int* id = mesh_indices.data();
				*id = indices[0];
				*(id + number + 1) = indices[1];
				*(id + number + 2) = indices[2];
				break;
			}
			case EDrawMode::TriangleStrip:
			{
				assert(number + 2 < mesh_indices.size());
				unsigned int* id = mesh_indices.data() + number;
				*(id + 0) = indices[0];
				*(id + 1) = indices[1];
				*(id + 2) = indices[2];
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

			// Normal data
			std::vector<glm::vec3>& normal_data = outNormals.getData();

			// Reset normal data so we can accumulate data into it. Note that this is a memset
			// instead of a loop to improve performance
			std::memset(normal_data.data(), 0, sizeof(glm::vec3) * normal_data.size());

			// Accumulate all normals into the normals array
			glm::vec3* normal_data_ptr = normal_data.data();

			// Go over all triangles in all triangle shapes
			TriangleIterator iterator(meshInstance);
			while (!iterator.isDone())
			{
				Triangle triangle = iterator.next();

				const TriangleData<glm::vec3>& triangleData = triangle.getVertexData(positions);
				glm::vec3 normal = glm::cross((triangleData.first() - triangleData.second()), (triangleData.first() - triangleData.third()));

				normal_data_ptr[triangle.firstIndex()] += normal;
				normal_data_ptr[triangle.secondIndex()] += normal;
				normal_data_ptr[triangle.thirdIndex()] += normal;
			}

			// Normalize to deal with shared vertices
			for (glm::vec3& normal : normal_data)
				normal = glm::normalize(normal);
		}


		void reverseWindingOrder(MeshInstance& mesh)
		{
			TriangleIterator iterator(mesh);
			while (!iterator.isDone())
			{
				Triangle triangle = iterator.next();
				Triangle::IndexArray indices = triangle.indices();
				std::swap(indices[0], indices[2]);

				int shapeIndex = triangle.getShapeIndex();
				setTriangleIndices(mesh.getShape(shapeIndex), mesh.getDrawMode(), triangle.getTriangleIndex(), indices);
			}
		}


		void generateIndices(nap::MeshShape& shape, int vertexCount, int offset)
		{
			MeshShape::IndexList& indices = shape.getIndices();
			indices.resize(vertexCount);
			for (int vertex = 0; vertex < vertexCount; ++vertex)
				indices[vertex] = vertex + offset;
		}


		void computeConnectivity(const MeshInstance& mesh, MeshConnectivityMap& outConnectivityMap)
		{
			// Resize to number of indices
			outConnectivityMap.resize(mesh.getNumVertices());

			TriangleIterator iterator(mesh);
			while (!iterator.isDone())
			{
				Triangle triangle = iterator.next();
				outConnectivityMap[triangle.firstIndex()].emplace_back(triangle);
				outConnectivityMap[triangle.secondIndex()].emplace_back(triangle);
				outConnectivityMap[triangle.thirdIndex()].emplace_back(triangle);
			}
		}


		float computeTriangleArea(const TriangleData<glm::vec3>& vertices)
		{
			// use Heron's formula
			float a = glm::length(vertices.first()  - vertices.second());
			float b = glm::length(vertices.first()  - vertices.third());
			float c = glm::length(vertices.second() - vertices.third());
			float s = (a + b + c) / 2.0f;
			return sqrt(s*(s - a)*(s - b)*(s - c));
		}


		float computeArea(nap::MeshInstance& mesh, const nap::VertexAttribute<glm::vec3>& vertices, std::vector<float>& outList)
		{
			// Clear list
			outList.clear();
			outList.reserve(utility::getTriangleCount(mesh));
			
			// Iterate and compute area for each triangle in the mesh
			TriangleIterator iterator(mesh);
			float total = 0.0f;
			while (!iterator.isDone())
			{
				Triangle triangle = iterator.next();
				const TriangleData<glm::vec3>& triangleData = triangle.getVertexData<glm::vec3>(vertices);
				float v = computeTriangleArea(triangleData);
				total += v;
				outList.emplace_back(v);
			}
			return total;
		}


		bool intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const TriangleData<glm::vec3>& vertices, glm::vec3& outCoordinates)
		{
			glm::vec3 e1 = vertices.second() - vertices.first();
			glm::vec3 e2 = vertices.third()  - vertices.first();

			glm::vec3 tri_normal = glm::cross(e1, e2);
			if (dot(rayDirection, tri_normal) > 0.0f)
				return false;

			glm::vec3 p = glm::cross(rayDirection, e2);
			float a = glm::dot(e1, p);

			float epsilon = math::epsilon<float>();
			if (a < epsilon && a > -epsilon)
				return false;

			float f = 1.0f / a;
			glm::vec3 s = rayOrigin - vertices.first();
			outCoordinates.x = f * glm::dot(s, p);
			if (outCoordinates.x < 0.0f || outCoordinates.x > 1.0f)
				return false;

			glm::vec3 q = glm::cross(s, e1);
			outCoordinates.y = f * glm::dot(rayDirection, q);
			if (outCoordinates.y < 0.0f || outCoordinates.y + outCoordinates.x > 1.0f)
				return false;

			outCoordinates.z = f * glm::dot(e2, q);
			return outCoordinates.z >= 0.0f;
		}


		glm::vec3 computeBarycentric(const glm::vec3& point, const TriangleData<glm::vec3>& triangle)
		{
			glm::vec3 v0 = triangle.second() - triangle.first();
			glm::vec3 v1 = triangle.third()  - triangle.first();
			glm::vec3 v2 = point - triangle.first();
			float d00 = glm::dot(v0, v0);
			float d01 = glm::dot(v0, v1);
			float d11 = glm::dot(v1, v1);
			float d20 = glm::dot(v2, v0);
			float d21 = glm::dot(v2, v1);
			float denom = d00 * d11 - d01 * d01;
			glm::vec3 r;
			r[1] = (d11 * d20 - d01 * d21) / denom;
			r[2] = (d00 * d21 - d01 * d20) / denom;
			r[0] = 1.0f - r[1] - r[2];
			return r;
		}
	}
}
