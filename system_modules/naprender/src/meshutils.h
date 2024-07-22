/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "mesh.h"
#include "box.h"
#include "triangleiterator.h"
#include "renderglobals.h"

// External Includes
#include <utility/dllexport.h>

namespace nap
{
	namespace utility
	{
		// Binds all the points to a set of triangular faces
		using MeshConnectivityMap = std::vector<std::vector<Triangle>>;

		/**
		 * Returns if the mesh is of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
		 * @param meshInstance the mesh to check
		 * @return if the mesh is of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
		 */
		bool NAPAPI isTriangleMesh(const nap::MeshInstance& meshInstance);

		/**
		 * Returns the total number of triangles associated with a mesh.
		 * @param meshInstance the mesh to inspect.
		 * @return the total number of triangles associated with a mesh
		 */
		int NAPAPI getTriangleCount(const MeshInstance& meshInstance);

		/**
		* Computes the normal that is associated with a triangular face. The normal is weighted (not normalized)
		* This call asserts when the index is out of bounds.
		* @param vertices mesh vertices
		* @return the weighted (not normalized) normal associated with a face
		*/
		glm::vec3 NAPAPI computeTriangleNormal(const TriangleData<glm::vec3>& vertices);

		/**
		* Sets the vertex indices associated with a triangle.
		* Note that this function only works for meshes that are of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
		* This call asserts when the triangle number is out of bounds, the mesh has no indices or the draw mode is not of type triangle
		* @param mesh the mesh to get the indices from
		* @param drawMode mesh topology
		* @param number the triangle number to get the for
		* @param indices the new indices
		* @return if the triangle indices are valid
		*/
		void NAPAPI setTriangleIndices(nap::MeshShape& mesh, EDrawMode drawMode, int number, const std::array<int, 3>& indices);

		/**
		* Computes the bounding box of a mesh using its associated position data.
		* Note that indices are not considered. This call loops over all available
		* points regardless of whether if they're drawn or not
		* @tparam the data type of the position attribute
		* @param mesh the mesh to get the bounding box for
		* @param outBox the computed bounding box
		*/
		template <typename T>
		void computeBoundingBox(const MeshInstance& mesh, math::Box& outBox);

		/**
		* Computes the bounding box of a single shape within a mesh using its associated position data.
		* Note that the given shape must be part of the mesh.
		* @tparam the data type of the position attribute
		* @param mesh the mesh that contains position data
		* @param shape the shape to compute the bounding box for
		* @param outBox the computed bounding box
		*/
		template <typename T>
		void computeBoundingBox(const nap::MeshInstance& mesh, const nap::MeshShape& shape, math::Box& outBox);

		/**
		* Automatically re-computes all the normals of a mesh
		* When the mesh has indices the normal is computed based on connectivity
		* Meshes without indices receive the triangular face normal
		* @param mesh the triangular mesh
		* @param vertices the vertex position attribute
		* @param outNormals the recomputed normals, the normals have to be initialized and of the same length as the vertices
		*/
		void NAPAPI computeNormals(const nap::MeshInstance& mesh, const nap::VertexAttribute<glm::vec3>& vertices, nap::VertexAttribute<glm::vec3>& outNormals);

		/**
		* Reverses the winding order of all the triangle vertices in a mesh
		* When a triangle has vertices A, B, C the new order will be C, B, A
		* This call only works for triangle meshes that have indices associated with it
		* This call asserts when the mesh is not a triangular mesh or the mesh has no indices
		* @param mesh the mesh to reverse the index winding order for
		*/
		void NAPAPI reverseWindingOrder(nap::MeshInstance& mesh);

		/**
		* Generates a list of sequential indices from offset up to vertexCount + offset.
		* @param shape The shape to generate indices for.
		* @param vertexCount number of indices to generate.
		* @param loop an extra index is added at the end, pointing to the the first one. Useful when creating a line loop.
		* @param offset The first index value.
		*/
		void NAPAPI generateIndices(nap::MeshShape& shape, int vertexCount, bool loop = false, int offset = 0);

		/**
		* Builds a 'map' that binds points (mesh index values) to faces
		* The index in the array corresponds to a mesh vertex index (point).
		* This call only works for meshes that have indices. When the mesh does not have indices this call asserts
		* Try to avoid building the map regularly, it's a heavy operation
		* This call asserts when the mesh is not a triangular mesh or has no indices associated with it
		* @param mesh the mesh to get build the array from
		* @param outConnectivityMap the array that is populated with the triangles associated with a single index
		*/
		void NAPAPI computeConnectivity(const nap::MeshInstance& mesh, MeshConnectivityMap& outConnectivityMap);

		/**
		 * @param vertices the triangle vertex position data
		 * @return the area of a triangle
		 */
		float NAPAPI computeTriangleArea(const TriangleData<glm::vec3>& vertices);

		/**
		 * Computes the area of every triangle in the mesh and stores the results in outList. The total is returned
		 * This call assumes the mesh has position vertex attribute data!
		 * @param mesh the mesh that contains the triangles
		 * @param vertices the mesh vertex positions
		 * @param outList a vector that contains the individual area of every triangle in the mesh
		 * @return the total area of the mesh
		 */
		float NAPAPI computeArea(nap::MeshInstance& mesh, const nap::VertexAttribute<glm::vec3>& vertices, std::vector<float>& outList);

		/**
		 * Calculates the intersection of a ray and a triangle in 3 dimensions
		 * Based on the Moller Trumbore intersection algorithm: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
		 * Back-facing triangles relative to the ray direction are not considered
		 * @param rayOrigin the origin of the ray, often the world space position of a camera
		 * @param rayDirection the direction of the ray from it's origin
		 * @param vertices the triangle vertex positions
		 * @param outCoordinates barycentric coordinates of point of intersection, where z is the scalar factor for the ray.
		 * @return if the ray intersects the triangle
		 */
		bool NAPAPI intersect(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const TriangleData<glm::vec3>& vertices, glm::vec3& outCoordinates);

		/**
		* Computes the barycentric coordinates for a point with respect to a triangle
		* @param point the position of the point in respect to the triangle
		* @param triangle the vertices of the triangle
		*/
		glm::vec3 NAPAPI computeBarycentric(const glm::vec3& point, const TriangleData<glm::vec3>& triangle);

		/**
		* Interpolates triangle vertex values based on barycentric u and v coordinates
		* @param vertexValues the values associated with the triangle vertices
		* @param barycentricCoordinates the triangle barycentric coordinates (u,v,w)
		* @return the interpolated vertex attribute value
		*/
		template<typename T>
		T interpolateVertexAttr(const TriangleData<T>& vertexValues, const glm::vec3& barycentricCoordinates);


		//////////////////////////////////////////////////////////////////////////
		// Template Definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		T interpolateVertexAttr(const TriangleData<T>& vertexValues, const glm::vec3& coords)
		{
			return (vertexValues.first() * (1.0f - coords.x - coords.y)) + (vertexValues.second() * coords.x) + (vertexValues.third() * coords.y);
		}

		template <typename T>
		void computeBoundingBox(const MeshInstance& mesh, math::Box& outBox)
		{
			glm::vec3 min(math::max<float>());
			glm::vec3 max(math::min<float>());

			const nap::VertexAttribute<T>& positions = mesh.getAttribute<T>(vertexid::position);
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

		template <typename T>
		void computeBoundingBox(const nap::MeshInstance& mesh, const nap::MeshShape& shape, math::Box& outBox)
		{
			glm::vec3 min(math::max<float>());
			glm::vec3 max(math::min<float>());

			const nap::VertexAttribute<T>& positions = mesh.getAttribute<T>(vertexid::position);
			for (unsigned int i : shape.getIndices())
			{
				const auto& point = positions[i];
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
	}
}
