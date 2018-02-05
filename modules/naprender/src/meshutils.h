#pragma once

#include <utility/dllexport.h>
#include <mesh.h>
#include <box.h>
#include <triangleiterator.h>

namespace nap
{
	namespace utility
	{
		// Binds all the points to a set of triangular faces
		using MeshConnectivityMap = std::vector<std::vector<Triangle>>;

		/**
		* @param mesh the mesh to check
		* @return if the mesh contains triangles, is of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
		*/
		bool NAPAPI isTriangleMesh(const nap::MeshShape& shape);

		/**
		* @return the number of triangles associated with a mesh
		*/
		int NAPAPI getTriangleCount(const MeshInstance& mesh);

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
		* @param number the triangle number to get the for
		* @param indices the new indices
		* @return if the triangle indices are valid
		*/
		void NAPAPI setTriangleIndices(nap::MeshShape& mesh, int number, const std::array<int, 3>& indices);

		/**
		* Computes the bounding box of a mesh using its associated position data
		* Note that indices are not considered. This call loops over all available
		* points regardless of whether if they're drawn or not
		* @param mesh the mesh to get the bounding box for
		* @param outBox the computed bounding box
		*/
		void NAPAPI computeBoundingBox(const nap::MeshInstance& mesh, nap::math::Box& outBox);

		/**
		* Computes the bounding box of a mesh using its associated position data
		* Note that indices are not considered. This call loops over all available
		* points regardless of whether they're drawn or not
		* @param mesh the mesh to get the bounding box for
		* @return the computed bounding box
		*/
		math::Box NAPAPI computeBoundingBox(const nap::MeshInstance& mesh);

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
		* Generates a list of sequential indices from @offset op to @vertexCount + @offset.
		* @param shape The shape to generate indices for.
		* @param vertexCount The number of indices to generate.
		* @param offset The first index value.
		*/
		void NAPAPI generateIndices(nap::MeshShape& shape, int vertexCount, int offset = 0);

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
		* Calculates the intersection of a ray and a triangle in 3 dimensions
		* Based on the M�ller�Trumbore intersection algorithm: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
		* Back-facing triangles relative to the ray direction are not considered
		* @param rayOrigin the origin of the ray, often the world space position of a camera
		* @param rayDirection the direction of the ray from it's origin
		* @param indices the triangle indices
		* @param vertices the triangle vertex positions
		* @param outBaryPosition barycentric coordinates of point of intersection, where z is the scalar factor for the ray.
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
	}
}
