#pragma once

#include <utility/dllexport.h>
#include <mesh.h>
#include <box.h>

namespace nap
{
	/**
	 * @param mesh the mesh to check
	 * @return if the mesh contains triangles, is of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
	 */
	bool NAPAPI isTriangleMesh(const nap::MeshShape& shape);

	/**
	* Computes the normal that is associated with a triangular face. The normal is weighted (not normalized)
	* This call asserts when the index is out of bounds.
	* @param indices the indices of the triangle
	* @param vertices mesh vertex position buffer
	* @return the weighted (not normalized) normal associated with a face
	*/
	glm::vec3 NAPAPI computeTriangleNormal(const glm::ivec3& indices, const nap::VertexAttribute<glm::vec3>& vertices);

	/**
	* Sets the vertex indices associated with a triangle.
	* Note that this function only works for meshes that are of type: TRIANGLES, TRIANGLE_STRIP or TRIANGLE_FAN
	* This call asserts when the triangle number is out of bounds, the mesh has no indices or the draw mode is not of type triangle
	* @param mesh the mesh to get the indices from
	* @param number the triangle number to get the for
	* @param indices the new indices
	* @return if the triangle indices are valid
	*/
	void NAPAPI setTriangleIndices(nap::MeshShape& mesh, int number, glm::ivec3& indices);

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
}
